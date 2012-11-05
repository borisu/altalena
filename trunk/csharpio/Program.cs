using System;
using System.Collections.Generic;
using System.Text;
using ivrworx.interop;
using System.Net;
using System.Threading;

namespace csharpio
{
    class Program
    {
        static string GetLocalIP()
        {
            IPHostEntry host;
            string localIP = "?";

            host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (IPAddress ip in host.AddressList)
            {
                if (ip.AddressFamily.ToString() == "InterNetwork")
                {
                    localIP = ip.ToString();
                }
            }

            if (localIP == "?")
                throw new Exception("Cannot resolve local IP");

            return localIP;

        }

        static void test1()
        {
            IvrWORX x = new IvrWORX();
            x.Init("dotnet.json");


            StreamerSession streamer = new StreamerSession(x);
            streamer.Allocate(null,
                StreamerSession.RcvDeviceType.RCV_DEVICE_NONE,
                StreamerSession.SndDeviceType.SND_DEVICE_TYPE_FILE);

            SipCall sipCall = new SipCall(x);

            Credentials c = new Credentials();
            c.User = "  bob";
            c.Realm = "example.com";
            c.Password = "password";

            LinkedList<string> contactList =
                new LinkedList<string>();
            contactList.AddLast("sip:1234" + "@" + GetLocalIP());

            ApiErrorCode res = sipCall.StartRegistration(
                contactList,
                "sip:7777@10.116.24.21",
                c,
                15000);

            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("Registration failed err:" + res);


            res = sipCall.MakeCall("sip:6051@10.116.24.21", streamer.LocalOffer(), c, null, 15000);


            streamer.ModifyConnection(sipCall.RemoteOffer());
            streamer.PlayFile(@"c:\SOUNDS\greeting.wav", true, false);

            sipCall.HangupCall();



            Console.WriteLine("MakeCall res=" + res);
        }


        static void test2()
        {
            IvrWORX x = new IvrWORX();
            x.Init("dotnet.json");


            RtpProxySession streamerRtpSession = new RtpProxySession(x);
            AbstractOffer dummyOffer = new AbstractOffer();
            dummyOffer.Type = "application/sdp";
            dummyOffer.Body=@"v=0"                                           +"\n"
                            +"o=alice 2890844526 2890844526 IN IP4 0.0.0.0"  +"\n"
                            +"s="                                            +"\n"
                            +"c=IN IP4 0.0.0.0"                              +"\n"
                            +"t=0 0"                                         +"\n"
                            +"m=audio 0 RTP/AVP 0"                           +"\n"
                            +"a=rtpmap:0 PCMU/8000"                          +"\n\n";;

            ApiErrorCode res = streamerRtpSession.Allocate(dummyOffer);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(1) Allocated failed err:" + res);

            StreamerSession streamer = new StreamerSession(x);

            streamer.Allocate(streamerRtpSession.LocalOffer(),
                StreamerSession.RcvDeviceType.RCV_DEVICE_NONE,
                StreamerSession.SndDeviceType.SND_DEVICE_TYPE_FILE);

            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("StreamerSession Allocated failed err:" + res);

            streamerRtpSession.Modify(streamer.LocalOffer());
            

            RtpProxySession sipRtpSession = new RtpProxySession(x);
            sipRtpSession.Allocate(dummyOffer);


            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(2) Allocated failed err:" + res);

            SipCall sipCall = new SipCall(x);


            res = 
                sipCall.MakeCall("sip:6051@10.116.24.21", sipRtpSession.LocalOffer(), null, null, 15000);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("MakeCall failed err:" + res);

            sipRtpSession.Modify(sipCall.RemoteOffer());

            sipRtpSession.Bridge(streamerRtpSession, true);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("Bridge failed err:" + res);


            
            streamer.PlayFile(@"c:\SOUNDS\greeting.wav", true, false);

            sipCall.Dispose();

            Console.WriteLine("MakeCall res=" + res);
        }

        static void test3()
        {

            IvrWORX x = new IvrWORX();
            x.Init("dotnet.json");

            //
            // Allocate RTP endpoint for RTSP session.
            //
            RtpProxySession rtspRtpSession = new RtpProxySession(x);
            AbstractOffer dummyOffer = new AbstractOffer();
            dummyOffer.Type = "application/sdp";
            dummyOffer.Body = @"v=0" + "\n"
                            + "o=alice 2890844526 2890844526 IN IP4 0.0.0.0" + "\n"
                            + "s=" + "\n"
                            + "c=IN IP4 0.0.0.0" + "\n"
                            + "t=0 0" + "\n"
                            + "m=audio 0 RTP/AVP 8" + "\n"
                            + "a=rtpmap:0 PCMA/8000" + "\n\n"; ;

            ApiErrorCode res = rtspRtpSession.Allocate(dummyOffer);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(1) Allocated failed err:" + res);


            //
            // Set up RTSP session and update its RTP endpoint.
            // 
            RtspSession rtsp = new RtspSession(x);
            res = rtsp.Setup("rtsp://10.116.100.78/IvrScript.wav", rtspRtpSession.LocalOffer());
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtspSession Setup failed err:" + res);

            res = rtspRtpSession.Modify(rtsp.RemoteOffer());
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtspSession Modify failed err:" + res);


            //
            // Allocate sip RTP session
            // 
            RtpProxySession sipRtpSession = new RtpProxySession(x);
            res = sipRtpSession.Allocate(dummyOffer);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(1) Allocated failed err:" + res);


            //
            // Make call
            //
            SipCall sipCall = new SipCall(x);
            res = sipCall.MakeCall("sip:6051@10.116.24.21", sipRtpSession.LocalOffer(), null, null, 15000);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("MakeCall failed err:" + res);

            sipRtpSession.Modify(sipCall.RemoteOffer());
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("Modify failed err:" + res);

            rtspRtpSession.Bridge(sipRtpSession, false);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("Bridge failed err:" + res);



            res = rtsp.Play(0.0,0.0,1);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(1) Allocated failed err:" + res);

            Thread.Sleep(10000);


            sipCall.Dispose();

            Console.WriteLine("MakeCall res=" + res);

            
        }

        static void test4()
        {
            IvrWORX x = new IvrWORX();
            x.Init("dotnet.json");

            //
            // Allocate RTP endpoint for RTSP session.
            //
            RtpProxySession call1RtpSession = new RtpProxySession(x);
            AbstractOffer dummyOffer = new AbstractOffer();
            dummyOffer.Type = "application/sdp";
            dummyOffer.Body = @"v=0" + "\n"
                            + "o=alice 2890844526 2890844526 IN IP4 0.0.0.0" + "\n"
                            + "s=" + "\n"
                            + "c=IN IP4 0.0.0.0" + "\n"
                            + "t=0 0" + "\n"
                            + "m=audio 0 RTP/AVP 8" + "\n"
                            + "a=rtpmap:0 PCMA/8000" + "\n\n"; ;

            ApiErrorCode res = call1RtpSession.Allocate(dummyOffer);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(1) Allocated failed err:" + res);


            //
            // Allocate sip RTP session
            // 
            RtpProxySession call2RtpSession = new RtpProxySession(x);
            res = call2RtpSession.Allocate(dummyOffer);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("RtpProxySession(1) Allocated failed err:" + res);


            //
            // Make call1
            //
            SipCall sipCall1 = new SipCall(x);
            res = sipCall1.MakeCall("sip:6051@10.116.24.21", call1RtpSession.LocalOffer(), null, null, 15000);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("MakeCall failed err:" + res);


            call1RtpSession.Modify(sipCall1.RemoteOffer());
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("Modify failed err:" + res);

            //
            // Make call2
            //
            SipCall sipCall2 = new SipCall(x);
            res = sipCall2.MakeCall("sip:6050@10.116.24.21", call2RtpSession.LocalOffer(), null, null, 15000);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("MakeCall failed err:" + res);
            call2RtpSession.Modify(sipCall2.RemoteOffer());


            call1RtpSession.Bridge(call2RtpSession, true);
            if (res != ApiErrorCode.API_SUCCESS)
                throw new Exception("Bridge failed err:" + res);



            Thread.Sleep(30000);
            sipCall1.Dispose();
            sipCall2.Dispose();

            Console.WriteLine("MakeCall res=" + res);

          
        }

        static void test5()
        {

            IvrWORX x = new IvrWORX();
            x.Init("dotnet.json");

            MrcpSession sesion = 
                new MrcpSession(x);

            


        }
        
        
        static void Main(string[] args)
        {
            test5();
            
        }
       }
}



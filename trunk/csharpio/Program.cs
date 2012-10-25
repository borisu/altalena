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

        static void Main(string[] args)
        {
            test2();
            
        }
       }
}



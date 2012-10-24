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

        static void Main(string[] args)
        {

            IvrWORX x = new IvrWORX();
            x.Init("dotnet.json");

            
            StreamerSession streamer = new StreamerSession(x);
            streamer.Allocate(null,
                StreamerSession.RcvDeviceType.RCV_DEVICE_NONE,
                StreamerSession.SndDeviceType.SND_DEVICE_TYPE_FILE);

            SipCall sipCall = new SipCall(x);
            
            Credentials c = new Credentials();
            c.User = "bob";
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
        }
    }


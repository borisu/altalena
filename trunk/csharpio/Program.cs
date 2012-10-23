using System;
using System.Collections.Generic;
using System.Text;
using ivrworx.interop;
using System.Net;

namespace csharpio
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                string fileName = "dotnet.json";
                using (IvrWORX x = new IvrWORX())
                {
                    x.Init(fileName);

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

                    if (localIP=="?") 
                        throw new Exception("Cannot resolve local IP");


                    SipCall sc = new SipCall(x);

                    
                    AbstractOffer offer = new AbstractOffer();
                    offer.Type = "application/sdp";
                    offer.Body = String.Format(@"v=0
o=alice 2890844526 2890844526 IN IP4 {0}
s=
c=IN IP4 {0}   
t=0 0
m=audio 49170 RTP/AVP 0 8 97
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000
a=rtpmap:97 iLBC/8000
m=video 51372 RTP/AVP 31 32
a=rtpmap:31 H261/90000
a=rtpmap:32 MPV/90000",localIP);

                    
                    Credentials c = new Credentials();
                    c.User = "bob";
                    c.Realm = "example.com";
                    c.Password = "password";


                    ApiErrorCode res = sc.MakeCall("sip:6050@10.116.100.90", offer,c, null,15000);
                    Console.WriteLine("MakeCall res=" + res);
                }
            }
            catch (System.Exception e)
            {
                System.Console.Out.WriteLine(e);
            }
            
        }
    }
}

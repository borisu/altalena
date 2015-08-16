# ivrworx -  VoIP Application Prototyping And Testing Framework #


## Please provide us your feedback !!! ##

<img src='http://prettythingsflying.com/images/lua.png' alt='lua logo' /> <img src='http://prettythingsflying.com/images/phone.JPG' alt='phone icone' /> <img src='http://prettythingsflying.com/images/dotnet.jpeg' alt='.net logo' />


**ivrworx** is a simple modular VoIP [Lua](http://www.lua.org) API (and .NET from 1.0.12 - higly experimental) which intended for testing complicated VoIP networks. It should be used as an application glue between different media service providers. In some sense, it is like SIPP but provides more high level API, strives to include as many protocols as possible (from any provider you wish) and provides developer with full blown programming language interface.


**SDP** - ivrworx uses SDP as a Lingua Franca to orchestrate between different media components. This is really how the all media components should communicate with each other.

**Why Lua?** - Sincerely... Lua rocks!

## Goals of Project ##

- Provide developers with quick VoIP prototyping and testing tool.

- Modularity and extensibility.

- Fun for me as a developer and for every one who joins the project.

## Getting Started ##

Download the latest zip and run Setup.exe. Edit **conf.json"**myscript.lua**file and run**runsimple.bat**. See wiki pages for examples of different scenarios.**

This is simple example of making call:-
```
require "ivrworx"

caller = sipcall:new()

localoffer = [[v=0
o=ivrworx 123456 654321 IN IP4 10.0.0.1
s=A conversation
c=IN IP4 10.0.0.1
t=0 0
m=audio 6000 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000/1
a=fmtp:101 0-11

]]

caller:makecall{dest="sip:24001@10.0.0.2", timeout=15, sdp=localoffer}

```

C#

```
Console.WriteLine("Hello World!");
			
IvrWORX x = new IvrWORX();
x.Init("conf.json");
SipCall sc = new SipCall(x);
			
AbstractOffer dummyOffer = new AbstractOffer();
dummyOffer.Type = "application/sdp";
dummyOffer.Body=@"v=0"                                           +"\n"
            +"o=alice 2890844526 2890844526 IN IP4 0.0.0.0"  +"\n"
            +"s="                                            +"\n"
            +"c=IN IP4 0.0.0.0"                              +"\n"
            +"t=0 0"                                         +"\n"
            +"m=audio 0 RTP/AVP 0"                           +"\n"
            +"a=rtpmap:0 PCMU/8000"                          +"\n\n";;

ApiErrorCode res = sc.MakeCall("sip:6050@10.116.24.21", dummyOffer,null,null,15000);
Console.Write(">>>>>>>>>>>>>>>>>> res= "+ res + "\n");
sc.HangupCall();
```


### What are we working on now? ###

> <img src='http://prettythingsflying.com/images/bug.jpg' alt='bug icon' /> - Bug fixes

### Supporting Vista+ & XP clients only. ###

Sorry linux fans...


## Join The Project ##
A lot of functionality is missing. Please send an e-mail to [me](mailto:borisusun@gmail.com) if you want to join the project.

Or [donate](https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=6G73CTTQYQLMA&lc=IL&item_name=ivrworx&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHosted)
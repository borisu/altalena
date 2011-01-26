require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")


dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 0.0.0.0
t=0 0
m=audio 0 RTP/AVP 0
a=rtpmap:0 PCMU/8000
]]

--
-- set up first media call
--
r1 = rtpproxy:new();
success(r1:allocate{sdp=dummyoffer});
m = mrcpsession:new();
success(m:allocate{resource=iw.SYNTHESIZER, sdp=r1:localoffer()});

r2 = rtpproxy:new();
success(r2:allocate{sdp=dummyoffer});
caller = sipcall:new();
success(caller:makecall{dest="sip:24001@192.168.150.3", sdp=r2:localoffer()}); 
success(r2:modify{sdp=caller:remoteoffer()})


--
-- half duplex the calls
--
sreq=[[
<?xml version="1.0"?>								
<speak version="1.0" xmlns="http://www.w3.org/2001/10/synthesis" xml:lang="en-US">  
 <p>														    
   <s>Hello World</s>								
 </p>															
</speak>]]																

success(r1:bridge{other=r2})
iw.sleep(1000);
success(m:speak{sentence="Hello World Borkis!"});

caller:waitforhangup();











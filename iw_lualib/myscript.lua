require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== START ===");

dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 192.168.100.164
t=0 0
m=audio 60000 RTP/AVP 8 101
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000
]]

--
-- set up first media call
--
r1 = rtpproxy:new();
r1:allocate{sdp=dummyoffer};

c = sipcall:new();
c:makecall{dest = "sip:24001@192.168.150.3", sdp=r1:localoffer()}
c:waitforhangup();

l:loginfo("=== END ===");						














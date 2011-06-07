require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== START ===");

dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 192.168.100.164
t=0 0
m=audio 60000 RTP/AVP 0
a=rtpmap:0 PCMU/8000
]]

--
-- set up first media call
--
r1 = rtpproxy:new();
r1:allocate{sdp=dummyoffer};


l:loginfo("=== END ===");						














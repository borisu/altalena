require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

--
-- set up first media call
--
require "ivrworx"

l:loginfo("=== START ===");						

caller = sipcall:new()

s = streamer:new();
s:allocate{rcv=iw.SND_DEVICE_TYPE_SND_CARD_MIC, snd=iw.SND_DEVICE_TYPE_FILE}

caller = sipcall:new();
caller:makecall{dest="sip:24001@192.168.150.3", timeout=60,sdp=s:localoffer()}


s:modify{sdp=caller:remoteoffer()}

s:play{file="C:\\tada.wav", loop=true}l:loginfo("=== END ===");						













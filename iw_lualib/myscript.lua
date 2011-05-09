require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

--
-- set up first media call
--
require "ivrworx"

l:loginfo("=== START ===");						

s = sipcall:new()
s:subscribe{server="sip:freeswitch@192.168.100.242",package="lulu",interval=60,refresh=180}

l:loginfo("=== END ===");						














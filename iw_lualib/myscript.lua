require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

--
-- set up first media call
--
require "ivrworx"

l:loginfo("=== START ===");						

s = sipcall:new()
s:subscribe{
	server="sip:user1@192.168.100.67",
	package="presence",
	user="user1",
	password="1234",
	realm="192.168.100.67",
	interval=60,
	refresh=180, 
	timeout=300};
	
iw.wait(30000);

l:loginfo("=== END ===");						














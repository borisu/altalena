require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== START ===");

--
-- set registration session to receive events upon
--
r= sipcall:new();			
r:startregister{registrar="sip:user1@192.168.100.242:5080", 
                        username="user1", 
                        password="1234",
                        realm="192.168.100.242",
                        timeout=15};

s = sipcall:new()
s:subscribe{
	server="sip:user1@192.168.100.242:5080",
	package="presence", 
	user="user1",
	password="1234",
	realm="192.168.100.242",
	interval=60,
	refresh=180, 
	timeout=300};
	
x,y,z = s:waitfornotify{timeout=15}

l:loginfo(z);

l:loginfo("=== END ===");						














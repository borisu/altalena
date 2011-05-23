require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

--
-- set up first media call
--
require "ivrworx"

l:loginfo("=== START ===");			

r= sipcall:new();			
r:startregister{registrar="sip:user1@192.168.100.242:5080", 
                        username="user1", 
                        password="1234",
                        realm="192.168.100.242",
                        timeout=15};
                        
r:makecall()
                        
-- iw.sleep(30000);


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














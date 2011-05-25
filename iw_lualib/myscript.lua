require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

--
-- set up first media call
--
require "ivrworx"

l:loginfo("=== START ===");


local caller = sipcall:new()
caller:makecall{dest="sip:24001@192.168.150.3",  
				timeout=15,
				username="user1", 
				password="1234",
				realm="192.168.100.242"};
				
localoffer = [[v=0
o=alice 2890844526 2890844526 IN IP4 host.ivrworx.com
s=
c=IN IP4 192.168.150.3
t=0 0
m=audio 49170 RTP/AVP 0
a=rtpmap:0 PCMU/8000

]]

caller:answer{sdp=localoffer}

iw.sleep(30000);
if (true) then return; end;
			

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














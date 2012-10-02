require "ivrworx"

local l = assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== START ===");

--
-- set registration session to receive events upon
--
s = sipcall:new()
s:subscribe{
        server="sip:user1@10.116.100.76:5070",
        package="presence", 
        user="6166",
        password="6166",
        realm="10.116.100.76",
        interval=60,
        refresh=180, 
        timeout=300};
        
x,y,z = s:waitfornotify{timeout=15}

l:loginfo(z);

l:loginfo("=== END ===");          
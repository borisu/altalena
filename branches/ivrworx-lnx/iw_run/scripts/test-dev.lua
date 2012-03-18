-- 
-- ivrworx test application
--
logger   = assert(ivrworx.LOGGER)
conf     = assert(ivrworx.CONF)
incoming = assert(ivrworx.INCOMING)

---
--- Main
---
logger:loginfo("*** Welcome to client example application! Please change the script to suite your environment!  ***");

logger:loginfo("INCOMING - Call Answered");
incoming:answer();
ivrworx.sleep(2000);
logger:loginfo("INCOMING - Exiting");

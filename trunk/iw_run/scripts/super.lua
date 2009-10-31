---
--- Super script is run upon system startup
---

require "play_phrase"

--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--

logger = assert(ivrworx.LOGGER)
conf = assert(ivrworx.CONF)

logger:loginfo("super *** Welcome!");


--
-- Make the call and blind transfer it to another destination
--
call = ivrworx.createcall();
if (call == nil) then
 ivrworx.loginf("Error making call res:" .. res)
  return;
end
 
res = call:makecall("sip:6095@10.0.0.1:5060");

call:play("C:\\dev\\CosmoCom\\Universe_5\\5_1_0\\Install Packs\\Server Components\\Files\\Samples\\VCS\\IVR\\greeting.wav",false,false);

call:speak("124563");


--call:play("C:\\dev\\CosmoCom\\Universe_5\\5_1_0\\Install Packs\\Server Components\\Files\\Samples\\VCS\\IVR\\greeting.wav",
--false,false);

ivrworx.sleep(3000);
call:stopplay();
ivrworx.sleep(10000);

call:blindxfer("sip:24001@192.168.150.3:5060"); 
 
logger:loginfo("super *** Good Bye!");
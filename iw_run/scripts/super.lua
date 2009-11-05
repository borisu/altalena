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

if (true) then
 return;
end


--
-- Make the call and blind transfer it to another destination
--
call = ivrworx.createcall();
if (call == nil) then
 ivrworx.loginf("Error making call res:" .. res)
  return;
end

greeting = "C:\\dev\\CosmoCom\\Universe_5\\5_1_0\\Install Packs\\Server Components\\Files\\Samples\\VCS\\IVR\\greeting.wav"
 
assert( call:makecall("sip:6095@192.168.150.3:5060")	== ivrworx.API_SUCCESS);
assert( call:play(greeting ,true,false)					== ivrworx.API_SUCCESS);
assert( call:speak("Welcome to CCU application",true)	== ivrworx.API_SUCCESS);
assert( call:blindxfer("sip:24001@192.168.150.3:5060")	== ivrworx.API_SUCCESS); 

logger:loginfo("super *** Good Bye!");
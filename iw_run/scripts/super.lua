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


call1 = CallBridge:new()
call1 = nil;
collectgarbage("collect");



--
-- Make the call and blind transfer it to another destination
--



greeting = "C:\\dev\\CosmoCom\\Universe_5\\5_1_0\\Install Packs\\Server Components\\Files\\Samples\\VCS\\IVR\\greeting.wav"

call = ivrworx.createcall();
assert(call:makecall("sip:6001@192.168.100.67:5060")	    == ivrworx.API_SUCCESS);
assert(call:speak("Welcome to CCU application" ,true)== ivrworx.API_SUCCESS);

play_phrase.spell_number(call,"123");






--assert( call:speak("Welcome to CCU application",true)	== ivrworx.API_SUCCESS);
--assert( call:blindxfer("sip:6001@192.168.100.67:5060")	== ivrworx.API_SUCCESS); 



logger:loginfo("super *** Good Bye!");
---
--- Super script is run upon system startup
---
require "ivrworx"
require "play_phrase"


ivrworx.loginf("Super script started");

--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--

--
-- Make the call and blind transfer it to another destination
--
-- res,handle = ivrworx.make_call("sip:6095@10.0.0.1:5060");
-- if (res ~= API_SUCCESS) then
--	ivrworx.loginf("Error making call res:" .. res)
--   return;
-- else 	
-- 	ivrworx.loginf("Sucess, call handle:" .. handle);
--   ivrworx.blind_xfer(handle,"sip:10.0.0.2:5064");
-- end

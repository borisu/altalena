---
--- Super script is run upon system startup
---
require "ivrworx"
require "play_phrase"

--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--



-- env = assert(luasql.sqlite3());
-- con = assert(env:connect("C:\\altalena\\altalena\\solutions\\altalena_all\\debug\\voice.db"))
-- cur = assert(con:execute"SELECT * FROM users")

-- row = cur:fetch ({}, "a")
-- while row do
--   ivrworx.loginf(string.format("user: %s, paswword: %s", row.login, row.password))
--   -- reusing the table of results
--   row = cur:fetch (row, "a")
-- end
-- -- close everything
-- cur:close()
-- con:close()
-- env:close()



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

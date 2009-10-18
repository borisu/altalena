
logger   = assert(ivrworx.LOGGER)
conf     = assert(ivrworx.CONF)
incoming = assert(ivrworx.INCOMING)

--
-- Utility functions
--
function getdtmfs(prompt_mrcp, termination_digit, interdigit_timeout, max_digits, max_retries)

	local curr_dtmf   = ""
	local dtmf_buffer = ""
	local res         = ivrworx.API_TIMEOUT

	while ( curr_dtmf ~= termination_digit and string.len(dtmf_buffer) < max_digits) do

		--
		-- Wait for DTMF's 
		--
		res,curr_dtmf = incoming:waitfordtmf(handle,interdigit_timeout);
		
 		if (res == ivrworx.API_TIMEOUT) then
 			--
			-- If timeout play request again
			--
     		incoming:speak(prompt_mrcp,false) == ivrworx.API_SUCCESS;
     		
		else
			--
			-- Add received dtmf to the buffer and stop playing
			--
			
			incoming:stopspeak(handle); 
			dtmf_buffer = dtmf_buffer..dtmf;
		end;
		
		retries = retries+1 
	end
end



---
--- Main
---


logger:loginfo("welcome to Polly client");


env = assert(luasql.sqlite3());
conn_string = assert(conf:getstring("polly_db"));
con = assert(env:connect(conn_string));

logger:loginfo("db:"..conn_string.." open");

assert(incoming:answer() == ivrworx.API_SUCCESS);

ivrworx.sleep(200000);

mrcp = [[<?xml version=\"1.0\"?>
<speak>
  <paragraph>
    <sentence>Welcome to polly application.</sentence>
  </paragraph>
</speak>]]

incoming:speak(mrcp,true);

mrcp = [[<?xml version=\"1.0\"?>
<speak>
  <paragraph>
    <sentence>Please enter user number</sentence>
  </paragraph>
</speak>]]

incoming:speak(mrcp,false);

ivrworx.sleep(20000)



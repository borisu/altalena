---
--- Super script is run upon system startup
---
require "ivrworx"
require "play_phrase"

ivrworx.loginf("Super script started");

--
-- Make call
--
res,handle = ivrworx.make_call("sip:24001@10.0.0.1:5060");
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error making call res:" .. res)
	return
end

ivrworx.loginf("Sucess, call handle:" .. handle);

--
-- Play greetings
--
res = ivrworx.play(handle,"test\\welcome-to-ivrworx.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing welcome file - res:" .. res)
	return
end

--
-- Ask user to enter id
--
res = ivrworx.play(handle,"test\\press-your-id-number-followed-by-pound-key.wav",false,false);
playing = true;
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing `pres...` file - res:" .. res)
	return
end


local dtmf = ""
local dtmf_str = ""
local res = 0

while dtmf ~= "#" do

	--
	-- Wait for DTMF's for less than 20 sec.
	--
	res,int_dtmf = ivrworx.wait_for_dtmf(handle,20000);
	
	
	
 	if (res == API_TIMEOUT) then
 	--
	-- If timeout play request again
	--
     	res = ivrworx.play(handle,"test\\press-your-id-number-followed-by-pound-key.wav",true,false);
     	playing = true;
     	if (res ~= API_SUCCESS) then
			ivrworx.loginf("Error playing `pres...` file - res:" .. res)
			return
		end
    else
    --
	-- Add received dtmf to the buffer and stop playing
	--
		if (playing == true) then ivrworx.stop_play(handle); playing = false; end;
		dtmf = string.char(int_dtmf);
    	ivrworx.logdbg("received dtmf:"..dtmf);
		dtmf_str = dtmf_str..dtmf;
	end;
end


--
-- Replay id number to user
-- 
res = ivrworx.play(handle,"test\\your-id-number-is.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing `your-id...` file - res:" .. res)
	return
end

play_phrase.spell(handle,dtmf_str);

ivrworx.play(handle, "test\\goodbye.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing goodbye file - res:" .. res)
	return
end






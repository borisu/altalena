require "ivrworx";
require "play_phrase";

function on_hangup()
 ivrworx.logdbg("remote hangup detected for script ani:"..linein["ani"]..", dnis:"..linein["dnis"]);
end

--
-- This is only to test parallel execution in load (duplicates script logic)
--
--a = 1
-- ivrworx.run(function() a = 4 end )
-- ivrworx.loginf( "script>"..a)

handle = INCOMING;

ivrworx.answer();
ivrworx.logdbg("script started ani:"..linein["ani"]..", dnis:"..linein["dnis"]);

--
-- Play greetings
--
res = ivrworx.play(handle,"test\\welcome-to-ivrworx.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing welcome file - res:" .. res)
	return
end

if string.len(linein["ani"]) ~= 0 
then
	ivrworx.play(handle, "test\\your-ani-number-is.wav",true,false);
	play_phrase.spell(handle, linein["ani"]);
end 

if string.len(linein["dnis"]) ~= 0 
then
	ivrworx.play(handle, "test\\your-dnis-number-is.wav",true,false);
	play_phrase.spell(handle, linein["dnis"]);
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
     	ivrworx.play(handle,"test\\press-your-id-number-followed-by-pound-key.wav",true,false);
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

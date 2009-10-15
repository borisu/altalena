require "ivrworx";
require "play_phrase";

a = LoggerBridge:new();
a:loginfo("he he");
--
-- This functions will run when caller hangs up
-- 
function on_hangup()
 a:loginfo("remote hangup detected for script ani:"..linein["ani"]..", dnis:"..linein["dnis"]);
end


---
--- INCOMING is handle of incoming call 
---
handle = INCOMING;
a:loginfo("start incoming call");

--
-- Answer the calls (you may filter calls by ani)
--
res = ivrworx.answer();
if (res ~= API_SUCCESS) then
	a:loginfo("Error answering the call - res:" .. res);
	return;
end

mrcp = [[<?xml version=\"1.0\"?>
<speak>
  <paragraph>
    <sentence>Hello.</sentence>
  </paragraph>
</speak>]]

ivrworx.speak(INCOMING,mrcp);
ivrworx.wait(10000);

if (1)
then
return;
end


-- 
-- Small example of running long operation in 
-- separate thread (NOT in parallel).
--
-- a = 1
-- ivrworx.run(function() a = 4; os.execute("pause"); end )
-- ivrworx.loginf( "script>"..a)
--

a:loginfo("script started ani:"..linein["ani"]..", dnis:"..linein["dnis"]);

if string.len(linein["ani"]) ~= 0 
then
	ivrworx.play(handle, "test\\your-callerid-is.wav",true,false);
	play_phrase.spell(handle, linein["ani"]);
end 

--if string.len(linein["dnis"]) ~= 0 
--then
--	ivrworx.play(handle, "test\\your-callerid-is..wav",true,false);
--	play_phrase.spell(handle, linein["dnis"]);
--end 


--
-- Ask user to enter id
--
ivrworx.play(handle,"test\\vm-enter-num-to-call.wav",true,false);
playing = true;


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
     	res = ivrworx.play(handle,"test\\vm-enter-num-to-call.wav",true,false);
     	playing = true;
     	if (res ~= API_SUCCESS) then
			ivrworx.loginf("Error playing `pres...` file - res:" .. res)
			return
		end
    else
    --
	-- Add received dtmf to the buffer and stop playing
	--
		if (playing == true) then 
		  ivrworx.stop_play(handle); 
		  playing = false; 
		end;
		
		dtmf = string.char(int_dtmf);
    	ivrworx.logdbg("received dtmf:"..dtmf);
		dtmf_str = dtmf_str..dtmf;
		
	end;
end

--
-- Replay id number to user
-- 
res = ivrworx.play(handle,"test\\vm-extension.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing `your-id...` file - res:" .. res)
	return
end

play_phrase.spell(handle,dtmf_str);

res = ivrworx.play(handle,"test\\vm-isunavail.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing `your-id...` file - res:" .. res)
	return
end

ivrworx.play(handle, "test\\vm-goodbye.wav",true,false);
if (res ~= API_SUCCESS) then
	ivrworx.loginf("Error playing goodbye file - res:" .. res)
	return
end

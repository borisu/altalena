require "ivrworx";
require "play_phrase";
-- require "ivrworx_lanes"

local il = ivrworx_lanes

function on_hangup()
 ivrworx.logdbg("remote hangup detected for script ani:"..linein["ani"]..", dnis:"..linein["dnis"]);
end



--
-- This is only to test parallel execution in load (duplicates script logic)
--
a = 1
ivrworx.run(function() a = 4 end )
ivrworx.loginf( "script>"..a)

ivrworx.answer();
ivrworx.logdbg("script started ani:"..linein["ani"]..", dnis:"..linein["dnis"]);

ivrworx.play("test\\welcome-to-ivrworx.wav",true,false);

if string.len(linein["ani"]) ~= 0 
then
	ivrworx.play("test\\your-ani-number-is.wav",true,false);
	play_phrase.spell(linein["ani"]);
end 

if string.len(linein["dnis"]) ~= 0 
then
	ivrworx.play("test\\your-dnis-number-is.wav",true,false);
	play_phrase.spell(linein["dnis"]);
end 


ivrworx.play("test\\press-your-id-number-followed-by-pound-key.wav",true,false);

local dtmf = ""
local dtmf_str = ""
local res = 0

while dtmf ~= "#" do

	res,int_dtmf = ivrworx.wait_for_dtmf(20000);
	-- timeout
	
 	if (res == 3) then
     	ivrworx.play("test\\press-your-id-number-followed-by-pound-key.wav",true,false);
    else
		dtmf = string.char(int_dtmf);
    	ivrworx.logdbg("received dtmf:"..dtmf);
		dtmf_str = dtmf_str..dtmf;
	end;
end

ivrworx.play("test\\your-id-number-is.wav",true,false);
play_phrase.spell(dtmf_str);

ivrworx.play("test\\goodbye.wav",true,false);



                          


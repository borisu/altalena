require "ivrworx";
require "play_phrase";


res = ivrworx.answer();
if (res ~= 0) then return end;

res = ivrworx.send_dtmf("123");

ivrworx.play("C:\\SOLUTIONS\\altalena\\iw_run\\sounds\\silence_5s.wav",true,false)

res = play_phrase.play_number(1);
if (res ~= 0) then return end


if (res ~= 0) then return end;

res = play_phrase.play_number(1);
if (res ~= 0) then return end


ani = tonumber(linein["ani"]);
if (ani == nil)  then return end

                              
res = play_phrase.play_number(1);
if (res ~= 0) then return end




                          


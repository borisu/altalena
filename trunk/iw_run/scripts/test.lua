require "ivrworx";
require "play_phrase";


res = ivrworx.answer();
if (res ~= 0) then return end;

ivrworx.play("C:\\SOLUTIONS\\altalena\\iw_run\\sounds\\click.wav",false,false);

ivrworx.wait(5000)



                          


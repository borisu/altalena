
require 'play_phrase';

res = this.answer(this);
if (res ~= 0) then return end


res = this.play(this,"C:\\SOLUTIONS\\altalena\\iw_run\\sounds\\welcome.wav",false,true);
if (res ~= 0) then return end

res, dtmf = this.wait_for_dtmf(this,60000);


                          


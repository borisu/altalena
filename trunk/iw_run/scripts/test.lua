require "ivrworx";
require "play_phrase";

res = ivrworx.answer();
if (res ~= 0) then return end;

res = play_phrase.play_number(conf["dnis"]);
if (res ~= 0) then return end




                          


require "ivrworx";
require "play_phrase";


ivrworx.answer();

ivrworx.logdbg("script started ani:"..linein["ani"]..", dnis:"..linein["dnis"]);

ivrworx.wait_till_hangup();

play_phrase.play_number(1);

ivrworx.blind_xfer("sip:6096@192.168.150.3");



                          


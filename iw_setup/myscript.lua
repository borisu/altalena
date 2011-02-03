require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

--
-- set up first media call
--

caller = sipcall:new();
success(caller:makecall{dest="sip:24001@192.168.150.3"}); 

recog = mrcpsession:new();
success(recog:allocate{resource=iw.RECOGNIZER, sdp=caller:remoteoffer()});
success(caller:answer{sdp = recog:remoteoffer{resource=iw.RECOGNIZER}})

--synth = mrcpsession:new();
--success(synth:allocate{resource=iw.SYNTHESIZER, sdp=caller:remoteoffer()});


iw.sleep(1000);
--success(synth:speak{sentence="Please choose red pill, or blue pill"});

pillgrammar=[[
<?xml version="1.0" encoding="UTF-8"?>
<!-- the default grammar language is US English -->
<grammar xmlns="http://www.w3.org/2001/06/grammar"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
         xml:lang="en-US" version="1.0" root="pill">
  <rule id="pill">
    <one-of>
      <item>red</item>
      <item>blue</item>
    </one-of> 
  </rule> 
</grammar>
]]

res, answer = recog:recognize{grammar=pillgrammar,sync=true, 
						cancel_if_queue=true, 
						content_id="<grammar1-borisu@form-level.store>",
						content_type="application/srgs+xml",
						no_input_timeout=20000,
						confidence_threshhold = 0.9,	
						start_input_timers=true};
						
if (res ~= iw.API_SUCCESS) then caller:hangup() end;

l:loginfo("=== RECOGNIZED START ===");						
l:loginfo(answer);						
l:loginfo("=== RECOGNIZED END ===");						

caller:waitforhangup();











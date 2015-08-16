

mrcpsession. MRCP protocol implementation.

current mrcp implementation is built with UniMRCP stack. Please see UniMRCP configuration guide before running MRCP functionality.  The template configuration are packaged with ivrworx installer.


## `:new(...)` ##
_ctor_
creates mrcpsession object

## `:allocate(...)` ##
allocates mrcp session

| **named parameter** | **description** |
|:--------------------|:----------------|
| sdp                 | local side offer |
| resource            | MRCP resource identifier see the table below |
| timeout             | timeout for resource allocation |

| **resource id** | **description**|
|:----------------|:---------------|
| SYNTHESIZER     | MRCP synthesizer |
| RECOGNIZER      | MRCP recognizer|


## `:recognize(...)` ##
sends RECOGNIZE request to MRCP server

| **named parameter** | **description** |
|:--------------------|:----------------|
| grammar             | grammar put in body of request (may be uri, or any other body type) |
| sync                | wait for recognition result |

One may add different MRCP headers to the message

### generic parameters ###
| **named parameter**| **type** |
|:-------------------|:---------|
| proxy\_sync\_id    | string   |
| accept\_charset    | string   |
| content\_type      | string   |
| content\_id        | string   |
| content\_base      | string   |
| content\_encoding  | string   |
| content\_location  | string   |
| cache\_control     | string   |
| logging\_tag       | string   |
| accept             | string   |
| fetch\_timeout     | number   |
| set\_cookie        | string   |
| set\_cookie2       | string   |

### recognize params ###
| **named parameter**| **type** |
|:-------------------|:---------|
| confidence\_threshold | number   |
| sensitivity\_level | number   |
| speed\_vs\_accuracy | number   |
| n\_best\_list\_length | number   |
| no\_input\_timeout | number   |
| recognition\_timeout | number   |
| waveform\_uri      | string   |
| completion\_cause  | number   |
| recognizer\_context\_block | string   |
| start\_input\_timers | boolean  |
| speech\_complete\_timeout | number   |
| speech\_incomplete\_timeout | number   |
| dtmf\_interdigit\_timeout | number   |
| dtmf\_term\_timeout |number    |
| dtmf\_term\_char   | string   |
| failed\_uri        | string   |
| failed\_uri\_cause | number   |
| save\_waveform     | string   |
| new\_audio\_channel | bool     |
| speech\_language   | string   |
| input\_type        | string   |
| input\_waveform\_uri | string   |
| completion\_reason | string   |
| media\_type        | string   |
| ver\_buffer\_utterance | boolean  |
| recognition\_mode  | string   |
| cancel\_if\_queue  | boolean  |
| hotword\_max\_duration | number   |
| hotword\_min\_duration | number   |
| interpret\_text    | string   |
| dtmf\_buffer\_time | boolean  |
| clear\_dtmf\_buffer | boolean  |
| early\_no\_match   | boolean  |


## `:remoteoffer(...)` ##
remote offer

| **named parameter** | **description** |
|:--------------------|:----------------|
| resource            | channel resource id (see the table above)|

## `:localoffer(...)` ##
local offer

| **named parameter** | **description** |
|:--------------------|:----------------|
| resource            | channel resource id (see the table above)|

## `:speak(...)` ##
Sends MRCP speak request.

| **named parameter** | **description** |
|:--------------------|:----------------|
| sentence            | sentence to say |
| rawbody             | full body       |
| sync                | wait for completion flag  |


### synthesize parameters ###

| **named parameter**| **type** |
|:-------------------|:---------|
|jump\_size          | number   |
|kill\_on\_barge\_in | boolean  |
|speaker\_profile    | string   |
|completion\_cause   | number   |
|completion\_reason  | string   |
|voice\_param        | TBD      |
|prosody\_param      | TBD      |
|speech\_marker      | string   |
|speech\_language    | string   |
|fetch\_hint         | string   |
|audio\_fetch\_hint  | string   |
|failed\_uri         | string   |
|failed\_uri\_cause  | string   |
|speak\_restart      | boolean  |
|speak\_length       | TBD      |
|load\_lexicon       | boolean  |
|lexicon\_search\_order| string   |

## `:stopspeak(...)` ##
Sends MRCP STOP SPEAK request.

## `:waitforrecog(...)` ##
waits for recognition result
| **named parameter**| **type** |
|:-------------------|:---------|
|timeout             | timeout  |

## example:simple ##

Pay attention that caller will receive traffic from port different then sending to. That may be a problem with some clients as linphone, as they correct SDP data thinking the NAT is involved

```
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

synth = mrcpsession:new();
success(synth:allocate{resource=iw.SYNTHESIZER, sdp=caller:remoteoffer()});


iw.sleep(1000);
success(synth:speak{sentence="Please choose red pill, or blue pill"});

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
```
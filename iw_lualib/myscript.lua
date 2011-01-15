require "ivrworx"

local l		  = assert(iw.LOGGER, "assert:iw.LOGGER")
local caller  = sipcall:new()


localoffer = [[v=0
o=ivrworx 123456 654321 IN IP4 192.168.100.51
s=A conversation
c=IN IP4 192.168.100.51
t=0 0
m=audio 6000 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000/1
a=fmtp:101 0-11

]]

s = streamer:new();
s:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE}

caller = sipcall:new();
caller:makecall{dest="sip:24001@192.168.150.3", timeout=60,sdp=s:localoffer()}
s:modify{sdp=caller:remoteoffer()}
s:play{file="C:\Windows\Media\tada.wav"}
	

res, dtmf = caller:gatherdigits{pattern="%d#", timeout=15}
l:loginfo(res.."=res".."signal="..dtmf)

caller:hangup();

iw.sleep(5000);
os.exit(0);






os.exit(0);


---
--- sip call alocation
---
str = streamer:new();

local leg1 = sipcall:new()
local leg2 = sipcall:new()


leg1:makecall{dest="sip:24001@192.168.150.3", timeout=15}
leg2:makecall{dest="sip:6095@192.168.150.3", sdp=leg1:remoteoffer(), timeout=15}

xsdp = leg2:remoteoffer();

leg1:answer{type="sdp", offer=xsdp}
leg1:waitfordtmf{pattern="[?+]#" , interdigittimeout=3, timeout=5}

--leg1:sendinfo{offer="crazyinfo", type="crazytype"}

iw.sleep(10000);




if (true) then return end;










--
-- Allocate streamer connection
-- 
streamer = assert(StreamerBridge:new()) 
assert(streamer:allocate0() == iw.API_SUCCESS, "assert:streamer:allocate0");
l:loginfo("streamer:localcnx() = "..streamer:localcnx():address())

--
-- Allocate mrcp object
--
mrcp = assert(MrcpBridge:new());


--
-- Allocate sip call object
--



--
-- Create synthesizer channel
--
assert(mrcp:allocate {
	localcnx_ut		= streamer:localcnx(),
	format_str		= "PCMU", 
    resource_int	= iw.SYNTHESIZER, 
    timeout=1000} == iw.API_SUCCESS, "assert:mrcp:allocate(tts)");
	
--
-- Create recognizer channel
--
assert(mrcp:allocate {
	localcnx_ut		= streamer:localcnx(),
	format_str		= "PCMU", 
	resource_int	= iw.RECOGNIZER, 
	timeout=1000} == iw.API_SUCCESS, "assert:mrcp:allocate(str)");
	
--
-- Modify streamer remote end (make it stream to MRCP server)
--
assert(streamer:modify {
		remotecnx_ut = mrcp:remotecnx{resource_int = iw.SYNTHESIZER},
		format_str = "PCMU", 
		rcv_device = iw.RCV_DEVICE_FILE_REC_ID,
		snd_device = iw.SND_DEVICE_TYPE_SND_CARD_MIC} == iw.API_SUCCESS);

---
--- Send recognize request
---
mrcpbody = [[
<?xml version="1.0"?>
<!-- the default grammar language is US English -->
<grammar xmlns="http://www.w3.org/2001/06/grammar" xml:lang="en-US" version="1.0" mode="voice" root="myname">
  <rule id="myname">
    <ruleref special="GARBAGE"/>
    <one-of>
      <item>Boris</item>
      <item>Lisa</item>
      <item>Irusik</item>
    </one-of>
  </rule>
</grammar>

]];


speak_body=[[
<?xml version="1.0"?>
<speak version="1.0" xmlns="http://www.w3.org/2001/10/synthesis"
         xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
         xsi:schemaLocation="http://www.w3.org/2001/10/synthesis
                   http://www.w3.org/TR/speech-synthesis/synthesis.xsd"
         xml:lang="en-US">
  <p>
    <s>This is the first sentence of the paragraph.</s>
    <s>Here's another sentence.</s>
  </p>
</speak>

]]

l:loginfo("Sending propmpt")
assert(mrcp:speak{
	raw_body=speak_body,
	content_type = "application/synthesis+ssml", 
	sync=true} == iw.API_SUCCESS);


l:loginfo("Sending recognize")
assert(mrcp:recognize{
	body=mrcpbody, 
	no_input_timeout=50000,
	recognition_timeout = 15000,
	confidence_threshold = 0.87,
	cancel_if_queue = true,
	content_id   = "ivrworx1@form-level.store",
	content_type = "application/srgs+xml",
	timeout=15, 
	sync=false} == iw.API_SUCCESS);
	
l:loginfo("waiting for results")	
assert(mrcp:waitforrecogresult{timeout=15} == iw.API_SUCCESS);

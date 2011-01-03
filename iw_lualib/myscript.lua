require "ivrworx"

iw=assert(ivrworx)


local function success(x,y)
	if (x==nil or type(x) ~= "number" or x ~= iw.API_SUCCESS) then 
		print(y);
		error(y); 
	end;
	return x;
end

local function getopt(arg, options)
    local opt, optind = {}, 1
    local waiting

    for _,v in ipairs(arg) do
        if waiting then
            -- short option waiting for a value
            opt[waiting] = v
            optind = optind + 1
            waiting = nil
        elseif v == "-" then
            break
        elseif v:sub(1, 1) == "-" then
            optind = optind + 1
            if v == "--" then
                break
            elseif v:sub(1, 2) == "--" then
                -- long option
                local x = v:find("=", 1, true)
                if x then
                    opt[v:sub(3, x-1)] = v:sub(x+1)
                else
                    opt[v:sub(3)] = true
                end
            else
                -- short option
                local j, l = 2, #v
                while (j <= l) do
                    local t = v:sub(j, j)
                    local x = options:find(t, 1, true)
                    if t == ":" then
                        io.stderr:write(arg[0],": invalid option --'", t, "'\n")
                        opt["?"] = true
                    elseif x then
                        if options:sub(x+1, x+1) == ":" then
                            local w = v:sub(j+1)
                            if #w > 0 then
                                opt[t] = w
                                j = l
                            else
                                waiting = t
                            end
                        else
                            opt[t] = true
                        end
                    else
                        io.stderr:write(arg[0],": invalid option --'", t, "'\n")
                        opt["?"] = true
                    end
                    j = j + 1
                end
            end
        else
            break
        end
    end

    if waiting ~= nil then
        io.stderr:write(arg[0],": option requires an argument -- '",waiting,"'\n")
        opt[":"] = true
    end

    return opt, optind
end


local argtable = assert(getopt(arg,"conf"), "assert:no-conf-parse")
local conffile = assert(argtable.conf,"assert:no-conf")


success(iw.init(conffile), "assert:iw.init")
local l = assert(iw.LOGGER, "assert:iw.LOGGER")


---
--- sip call alocation
---
str = streamer:new();

local leg1 = sipcall:new()


leg1:makecall{dest="sip:24001@192.168.150.3", timeout=15, offer="kokoko", type="crazy"}
leg2:makecall{dest="sip:6095@192.168.150.3", sdp=leg1:remoteoffer(), timeout=15}

leg1:answer{sdp=leg2:remoteoffer()}

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

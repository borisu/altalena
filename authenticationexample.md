
```
require "ivrworx"

-- Print anything - including nested tables
function table_print (tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    for key, value in pairs (tt) do
      io.write(string.rep (" ", indent)) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        io.write(string.format("[%s] => table\n", tostring (key)));
        io.write(string.rep (" ", indent+4)) -- indent it
        io.write("(\n");
        table_print (value, indent + 7, done)
        io.write(string.rep (" ", indent+4)) -- indent it
        io.write(")\n");
      else
        io.write(string.format("[%s] => %s\n",
            tostring (key), tostring(value)))
      end
    end
  else
    io.write(tt .. "\n")
  end
end


function parseargs(s)
  local arg = {}
  string.gsub(s, "(%w+)=([\"'])(.-)%2", function (w, _, a)
    arg[w] = a
  end)
  return arg
end

function collect(s)
  local stack = {}
  local top = {}
  table.insert(stack, top)
  local ni,c,label,xarg, empty
  local i, j = 1, 1
  while true do
    ni,j,c,label,xarg, empty = string.find(s, "<(%/?)([%w:]+)(.-)(%/?)>", i)
    if not ni then break end
    local text = string.sub(s, i, ni-1)
    if not string.find(text, "^%s*$") then
      table.insert(top, text)
    end
    if empty == "/" then  -- empty element tag
      table.insert(top, {label=label, xarg=parseargs(xarg), empty=1})
    elseif c == "" then   -- start tag
      top = {label=label, xarg=parseargs(xarg)}
      table.insert(stack, top)   -- new level
    else  -- end tag
      local toclose = table.remove(stack)  -- remove top
      top = stack[#stack]
      if #stack < 1 then
        error("nothing to close with "..label)
      end
      if toclose.label ~= label then
        error("trying to close "..toclose.label.." with "..label)
      end
      table.insert(top, toclose)
    end
    i = j+1
  end
  local text = string.sub(s, i)
  if not string.find(text, "^%s*$") then
    table.insert(stack[#stack], text)
  end
  if #stack > 1 then
    error("unclosed "..stack[#stack].label)
  end
  return stack[1]
end

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

function getinputfromresult(xmlres)

local t = collect(xmlres)

if (t==nil
	or t[2] == nil
	or t[2][1] == nil
	or t[2][1][1] == nil
	or t[2][1][1] == nil
	or t[2][1][1]["label"] ~= "input") then
		l:loginfo("error parsing XML answer");
	return nil;
end

return t[2][1][1][1];

end




--
-- 4 digits grammar
--
pingr=[[
<?xml version="1.0"?>

	<grammar mode="dtmf" version="1.0" xml:lang="en-US"
			 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
			 xsi:schemaLocation="http://www.w3.org/2001/06/grammar
								 http://www.w3.org/TR/speech-grammar/grammar.xsd"
			 xmlns="http://www.w3.org/2001/06/grammar" root="pin">

	<rule id="digit">
	 <one-of>
	   <item> 0 </item>
	   <item> 1 </item>
	   <item> 2 </item>
	   <item> 3 </item>
	   <item> 4 </item>
	   <item> 5 </item>
	   <item> 6 </item>
	   <item> 7 </item>
	   <item> 8 </item>
	   <item> 9 </item>
	 </one-of>
	</rule>

	<rule id="pin" scope="public">
		 <item repeat="4"><ruleref uri="#digit"/></item>
	</rule>

</grammar>
]]


function polly_the_parrort_application()

	l:loginfo("=== Polly The Parrot ===")

	--
	-- accept the call
	--
	l:loginfo("Waiting for the caller")
	local caller = sipcall:new()
	local res = caller:accept{service="sip,resip", timeout=30000};
	if (res ~= iw.API_SUCCESS) then
		l:loginfo("polly:error accepting calls, res:"..res);
		os.exit(0)
	end;


	--
	-- create MRCP session and send SDP answer
	-- to the caller
	--
	local recog = mrcpsession:new();
	res = recog:allocate{resource=iw.RECOGNIZER, sdp=caller:remoteoffer()};
	if (res ~= iw.API_SUCCESS) then
		caller:hangup();
		l:loginfo("polly:error allocating mrcp recognizer channel="..res);
		return;
	end;

	res = caller:answer{sdp = recog:remoteoffer{resource=iw.RECOGNIZER}}
	if (res ~= iw.API_SUCCESS) then
		caller:hangup();
		l:loginfo("polly:error answering the caller res:"..res);
		return;
	end;

	local synth = mrcpsession:new();
	res = synth:allocate{resource=iw.SYNTHESIZER, sdp=caller:remoteoffer()};
	if (res ~= iw.API_SUCCESS) then
	    caller:hangup();
		l:loginfo("polly:error allocating mrcp synthesizer channel="..res);
		return;
	end;



	res = synth:speak{sentence="Please enter your user identifier",async=true};
	if (res ~= iw.API_SUCCESS) then
		caller:hangup();
		l:loginfo("polly:error playing prompt res:"..res);
		return;
	end;



	local answer
	res, answer = recog:recognize{grammar=pingr,
								sync=true,
								cancel_if_queue=true,
								content_id="<grammar1-borisu@form-level.store>",
								content_type="application/srgs+xml",
								no_input_timeout=20000,
								confidence_threshhold = 0.9,
								start_input_timers=true};

	if (res ~= iw.API_SUCCESS or answer == "") then
		l:loginfo("polly:error recognition :"..res);
		synth:speak{sentence="Sorry, cannot understand your input"};
		caller:hangup();
		return;
	end;



	l:loginfo("username answer:\n" .. answer);

	local username = getinputfromresult(answer);

	l:loginfo("username dtmfinput:\n" .. username);

	if (username == nil) then
		l:loginfo("polly:error recognition :"..res);
		synth:speak{sentence="Sorry, cannot understand your input"};
		caller:hangup();
		return;
	end;


	res = synth:speak{sentence="Please enter your password",sync=false};
	if (res ~= iw.API_SUCCESS) then
		caller:hangup();
		l:loginfo("polly:error playing prompt res:"..res);
		return;
	end;

	res, answer = recog:recognize{grammar=pingr,
								sync=true,
								cancel_if_queue=true,
								content_id="<grammar1-borisu@form-level.store>",
								content_type="application/srgs+xml",
								no_input_timeout=20000,
								confidence_threshhold = 0.9,
								start_input_timers=true};

	if (res ~= iw.API_SUCCESS or answer == "") then
		l:loginfo("polly:error recognition :"..res);
		synth:speak{sentence="Sorry, cannot understand your input"};
		caller:hangup();
		return;
	end;



	l:loginfo("password answer:\n" .. answer);

	local password = getinputfromresult(answer);

	if (password == nil) then
		l:loginfo("polly:error recognition :"..res);
		synth:speak{sentence="Sorry, cannot understand your input"};
		caller:hangup();
		return;
	end;

	l:loginfo("username dtmfinput:\n" .. password);


	if (password == "1 2 3 4" and username == "1 2 3 4") then
		res = synth:speak{sentence="Welcome to the system", sync=true};
	else
		res = synth:speak{sentence="Wrong username or password", sync=true};

	end



	caller:hangup();

end

while (true) do

	polly_the_parrort_application()

end
```
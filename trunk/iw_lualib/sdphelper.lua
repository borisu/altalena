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


local function deepcopy(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

local staticsdp = {}

staticsdp[0] =  {ptype=0,  ename="PCMU", crate=8000}
staticsdp[3] =  {ptype=3,  ename="GSM", crate=8000}
staticsdp[4] =  {ptype=4,  ename="G723",crate=8000}
staticsdp[5] =  {ptype=5,  ename="DVI4",crate=8000}
staticsdp[6] =  {ptype=6,  ename="DVI4",crate=16000}
staticsdp[7] =  {ptype=7,  ename="LPC",crate=8000}
staticsdp[8] =  {ptype=8,  ename="PCMA",crate=8000}
staticsdp[9] =  {ptype=9,  ename="G722",crate=8000}
staticsdp[10] = {ptype=10, ename="L16",crate=44100}
staticsdp[11] = {ptype=11, ename="L16",crate=44100}
staticsdp[12] = {ptype=12, ename="QCELP",crate=8000}
staticsdp[13] = {ptype=13, ename="CN",crate=8000}
staticsdp[14] = {ptype=14, ename="MPA",crate=8000}
staticsdp[15] = {ptype=15, ename="G728",crate=8000}
staticsdp[16] = {ptype=16, ename="DVI4",crate=11025}
staticsdp[17] = {ptype=17, ename="DVI4",crate=22050}
staticsdp[18] = {ptype=18, ename="G729",crate=8000}


function parse_sdp(sdp)

	local res = {}

	-- key=value parser iterator
	local i  = string.gmatch(sdp,"(%w+)=([^\n\r]*)")
	local k,v = i()

	-- protocol version
	if (k ~= "v") then return nil end
	res["v"] = v;
	k,v = i()

	-- originator and session identifier
	if (k ~= "o") then return nil end
	res["o"] = v;
	k,v = i()

	-- session name
	if (k ~= "s") then return nil end
	res["s"] = v;
	k,v = i()

	-- session information
	if (k == "i") then
		res["i"] = v;
		k,v = i()
	end

	-- URI of description
	if (k == "u") then
		res["u"] = v;
		k,v = i()
	end

	-- email address
	if (k == "e") then
		res["e"] = v;
		k,v = i()
	end

	-- phone number
	if (k == "p") then
		res["p"] = v;
		k,v = i()
	end

	-- connection
	if (k == "c") then
		res["c"] = v;
		k,v = i()
	end

	--
	-- b=X-YZ:128
	-- b=X-Y1:128
	--
	-- =>res["b"] ={ "X-YZ"="128", "X-Y1"="128"}
	--
	res["b"],k,v = parse_bandwith(k,v,i)

	-- t=2873397496 2873404696
	-- r=1
	-- r=2
	-- r=3
	--
	-- =>res["t"] ={ "t"="2873397496 2873404696", "r"={1,2,3}}
	--

	res["t"],k,v = parse_time_description(k,v,i)


	-- time zone adjustments
	if (k == "z") then
		res["z"] = v;
		k,v = i()
	end

	-- encryption key
	if (k == "k") then
		res["k"] = v;
		k,v = i()
	end



	--a=inactive
	--a=xyz
	--a=rtpmap:99 h263-1998/90000
	--
	-- res["a"] = {"attributes" = { "inactive","xyz","rtpmap:99 h263-1998/90000"},
	--			   "rtpmap"		= { {"ptype"=99, "ename"="h263-1998", "crate"=9000}}

	res["a"],k,v = parse_attributes(k,v,i)


	-- m=audio 49170 RTP/AVP 0
	-- a=inactive
	-- m=video 51372 RTP/AVP 99
	-- a=rtpmap:99 h263-1998/90000
	--
	-- =>res["m"] ={
	-- 			{"medium"="audio", "port"="49170", "codecs"={"0"},"a"={...}},
	-- 			{"medium"="video", "port"="51372", "codecs"={"99"}, "a"={...} }
	--		}
	--

	--
	res["m"],k,v = parse_media_descriptors(k,v,i)




	return res;



end

function parse_bandwith(k,v,i)

	local res = nil

	-- zero or more bandwidth information lines
	while (k == "b") do

		if res == nil then res = {}; end;

		local myi = string.gmatch(v,"([^:]+):([^\n\r]*)")
		local bwtype,bandwidth  = myi()
		res[bwtype] = bandwidth
		k,v = i()

	end

	return res,k,v

end

function parse_time_description(k,v,i)

	local res = nil

	-- time the session is active
	if (k == "t") then

		if res == nil then res = {}; end;
		local myi = string.gmatch(v,"([^\n\r]*)")
		local val  = myi()
		res["t"] = val
		k,v = i()

	else
		return res,k,v
	end

	-- zero or more repeat times
	while ( k == "r") do
	  if (res["r"] == nil) then res["r"] = {}; end;
	  table.insert(res["r"],v);
	  k,v = i()
	end

	return res,k,v

end

function parse_attributes(k,v,i)

local res = nil
local rtpmap = nil


	while (k == "a") do

		if res == nil then res = {}; res["attributes"] = {}; end;
		table.insert(res["attributes"],v)


		if string.find(v,"rtpmap:") ~= nil then

		 if (rtpmap == nil) then
			rtpmap = {};
			res["rtpmap"] = rtpmap;
		 end;

		 local myi = string.gmatch(v,"rtpmap:(%d+) ([%w_%-]+)/(%d+)")
		 local ptype, ename, crate = myi();
		 -- override static definitions
		 rtpmap[ptype] = {ptype=ptype, ename=ename, crate=crate};

		end

		k,v = i()

	end

	return res,k,v

end

function parse_media_descriptors(k,v,i)
	local res = nil
	-- time the session is active
	while (k == "m") do

		if res == nil then res = {}; end;

		local myi = string.gmatch(v,"([^\ ]*) ([^\ ]*) ([^\ ]*) (.*)")
		local media, port, protocol, list1 = myi();



		thismedia = {}
		thismedia ["medium"] = media
		thismedia ["port"] = port
		thismedia ["protocol"] = protocol
		thismedia ["codecs"] = {}

		local myi1 = string.gmatch(list1 , "(%w+)[ ]?")
		for codec in myi1 do
			table.insert(thismedia ["codecs"], codec)
		end

		k,v = i()

		-- session information
		if (k == "i") then
			thismedia["i"] = v;
			k,v = i()
		end

		-- connection
		if (k == "c") then
			thismedia["c"] = v;
			k,v = i()
		end

		res["b"],k,v = parse_bandwith(k,v,i)

		-- encryption key
		if (k == "k") then
			thismedia["k"] = v;
			k,v = i()
		end


		thismedia["a"],k,v = parse_attributes(k,v,i)

		table.insert(res,thismedia);
	end

	return res,k,v

end


function convert_bandwith(bwtree)

	local res = "";

	if (bwtree == nil) then return ""; end;

	for key,value in pairs(bwtree) do
		res=res.."b="..key..":"..value.."\n";
	end

	return res;

end

function conver_time_descriptors(ttree)

	local res = "";

	if (ttree == nil) then return ""; end;

	res=res.."t="..ttree["t"].."\n";

	if (ttree["r"] ~= nil) then

		for key,value in pairs(ttree["r"]) do
			res=res.."r="..value.."\n";
		end

	end

	return res;

end

function convert_attributes(atree)

	local res = "";

	if (atree == nil or atree["attributes"] == nil) then return ""; end;

	for key,value in pairs(atree["attributes"]) do
			res=res.."a="..value.."\n";
	end


	return res;

end

function convert_media_descriptors(mtree)

	print("=================================================")
   table_print(mtree)
   print("=================================================")

	local res = "";

	if (mtree == nil) then return ""; end;

	for key,value in pairs(mtree) do

		mediumtree = value;

		res=res.."m="..mediumtree["medium"].." "..mediumtree["port"].." "..mediumtree["protocol"].." ";

		for key1,value1 in pairs(mediumtree["codecs"]) do
			res= res..value1.." ";
		end
		res=res.."\n"


		res=res..convert_attributes(mediumtree["a"]);
	end


	return res;

end


function convert_sdp(parsedsdp)

	local res = ""

	-- protocol version
	if (parsedsdp["v"] ~= nil) then res=res.."v="..parsedsdp["v"].."\n"; end;

	-- parsedsdp and session identifier
	if (parsedsdp["o"] ~= nil) then res=res.."o="..parsedsdp["o"].."\n"; end;

	-- session name
	if (parsedsdp["s"] ~= nil) then res=res.."s="..parsedsdp["s"].."\n"; end;

	-- session information
	if (parsedsdp["i"] ~= nil) then res=res.."i="..parsedsdp["i"].."\n"; end;

	-- URI of description
	if (parsedsdp["u"] ~= nil) then res=res.."u="..parsedsdp["u"].."\n"; end;

	-- email address
	if (parsedsdp["e"] ~= nil) then res=res.."e="..parsedsdp["e"].."\n"; end;

	-- phone number
	if (parsedsdp["p"] ~= nil) then res=res.."p="..parsedsdp["p"].."\n"; end;

	-- connection
	if (parsedsdp["c"] ~= nil) then res=res.."c="..parsedsdp["c"].."\n"; end;

	res = res..convert_bandwith(parsedsdp["b"]);

	res = res..conver_time_descriptors(parsedsdp["t"]);

	-- time zone adjustments
	if (parsedsdp["z"] ~= nil) then res=res.."z="..parsedsdp["z"].."\n"; end;

	-- encryption key
	if (parsedsdp["k"] ~= nil) then res=res.."k="..parsedsdp["k"].."\n"; end;

	res = res..convert_attributes(parsedsdp["a"]);

	res = res..convert_media_descriptors(parsedsdp["m"]);

	return res;

end


function find_first_audio_medium(mtree)




	for key,value in pairs(mtree) do
		if (value["medium"] == "audio" and
		   (value["protocol"] == "RTP" or value["protocol"] == "RTP/AVP")) then
			return value;
		end
	end
end

function get_codec_info(pt, atree, global_atree)

	local res = pt;


	-- firstly try to look codec info in attributes of the medium itself
	if (atree ~= nil and atree["rtpmap"] ~= nil  and atree["rtpmap"][pt] ~= nil) then
		res = atree["rtpmap"][pt]
		return res;
	end

	-- firstly try to look codec info in global SDP attributes
	if (global_atree ~= nil and global_atree["rtpmap"] ~= nil  and global_atree["rtpmap"][pt] ~= nil) then
		res = global_atree["rtpmap"][pt]
		return res;
	end

	-- last chance is static SDP definitons
	if (staticsdp[pt] ~= nil) then
		res = staticsdp[pt]
	end


	-- assume the codec is give in its already resolved state
	return pt

end

--
-- finds first matching codec in first audio medium
--

function negotiate_audio_sdp(offer1, offer2)


	if (offer1["m"] == nil or offer2["m"] == nil) then
		return nil
	end




	local m1 = find_first_audio_medium(offer1["m"])
	local m2 = find_first_audio_medium(offer2["m"])



	if (m1==nil or m2==nil) then
		return nil
	end

	local commoncodecs={};

	local ci1,ci2;
	for key,value in pairs(m1["codecs"]) do
		ci1 = get_codec_info(value, m1["a"], offer1["a"])
		for key,value in pairs(m2["codecs"]) do
			ci2 = get_codec_info(value, m2["a"], offer2["a"])
			if (ci1["ename"] == ci2["ename"]) then table.insert(commoncodecs,ci1); end;
		end
	end



	res = deepcopy(offer1);

	local medium = {}


	medium["medium"] 	= m1["medium"]
	medium["port"] 	    = m1["port"]
	medium["protocol"]  = m1["protocol"]
	medium["codecs"] 	= {ci1["ptype"]}
	medium["a"] 		= deepcopy(m1["a"]);

	res["m"] = {};

	table.insert(res["m"],medium);

	return res;

end




--[[
local remoteoffer = \[\[v=0
 o=jdoe 2890844526 2890842807 IN IP4 10.47.16.5
 s=SDP Seminar
 i=A Seminar on the session description protocol
 u=http://www.example.com/seminars/sdp.pdf
 e=j.doe@example.com (Jane Doe)
 c=IN IP4 224.2.17.12/127
 t=2873397496 2873404696
 a=recvonly
 m=audio 49170 RTP/AVP 0
 a=rtpmap:0 PCMU/8000/1
 m=video 51372 RTP/AVP 99
 a=rtpmap:99 h263-1998/90000
  \]\]

local localoffer = \[\[v=0
o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4
s=Sd seminar
i=A seminar on the session description protocol
u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.01.ps
e=M.Handley@cs.ucl.ac.uk (Mark Handley)
c=IN IP4 224.2.17.12/127
b=X-YZ:128
b=X-Y1:128
t=2873397496 2873404696
r=1
r=2
r=3
z=zzzz
k=kkkkk
a=recvonly
a=rtpmap:99 h263-1998/90000
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000
m=audio 3456 VAT 0
m=audio 3456 RTP/AVP 0
m=video 2232 RTP H261
m=whiteboard 32416 UDP WB
a=orient:portrait

\]\]



parsedlocal = parse_sdp(localoffer)
parsedremote = parse_sdp(remoteoffer)



local nsdp = negotiate_audio_sdp(parsedremote, parsedlocal)

if (nsdp == nil) then
 print ("error negotiating!!!!");
 return
end

print("=================================================")
table_print(nsdp)
print("=================================================")

print(convert_sdp(nsdp))


--]]



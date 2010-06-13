--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--

logger = assert(ivrworx.LOGGER)
conf   = assert(ivrworx.CONF)

logger:loginfo("*** CONF START ***");

port = 6000

function creare_sdp(address)

port = port+2;

sdp = 
[[v=0
o=user1 53655765 2353687637 IN IP4 ]]..address..[[
s=-
c=IN IP4 ]]..address..[[
t=0 0
m=audio ]]..port..[[ RTP/AVP 0
a=rtpmap:0 PCMU/8000

]]);

end


function main()

	---
	--- CREATE CCL LEG
	--- 
	local hpceaddress  = "192.168.100.177";
	local localaddress = "192.168.100.44";
	local ccl = assert(ivrworx.createsession("mscml"));
	 
	   
	logger:loginfo("Creating conference ...");
	res	 = ccl:setup(
	"sip:conf=conf1@"..hpceaddress..":5060",
[[<?xml version="1.0" encoding="utf-8"?>
	   <MediaServerControl version="1.0">
		 <request>
		   <configure_conference reservedtalkers="3" reserveconfmedia="yes">
			 <subscribe>
			   <events>
				 <activetalkers report="yes" interval="60s"/>
			   </events>
			 </subscribe>
		   </configure_conference>
		 </request>
</MediaServerControl>]]);
	assert(res == ivrworx.API_SUCCESS);

	--
	-- ADD FIRST PARTICIPANT
	--
	logger:loginfo("Adding first participant p1 ...");
	local p1 = assert(ivrworx.createsession("sip"));
	res	= p1:makecall(
	"sip:conf=conf1@"..hpceaddress..":5060", 
[[v=0
o=user1 53655765 2353687637 IN IP4]]..localaddress..[[
s=-
c=IN IP4 192.168.100.44
t=0 0
m=audio 6000 RTP/AVP 0
a=rtpmap:0 PCMU/8000

]]);
	
	if (res <> ivrworx.API_SUCCESS) return res;
	
	return ivrworx.API_SUCCESS;

end

while (true) do
 logger:loginfo("Enter key..."); 
 age = io.stdin:read'*l';
 main()
 
end 

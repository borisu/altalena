--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--

logger = assert(ivrworx.LOGGER)
conf   = assert(ivrworx.CONF)

logger:loginfo("*** CONF START ***");

ccl = assert(ivrworx.createsession("mscml"));

setup_conf = [[
<?xml version="1.0" encoding="utf-8"?>
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
</MediaServerControl>
]];
   
hpceaddress = "192.168.100.177";    
   
logger:loginfo("Creating conference ...");
res	 = ccl:setup("sip:conf=conf1@"..hpceaddress..":5060",setup_conf);
assert(res == ivrworx.API_SUCCESS);

--
-- ADD FIRST PARTICIPANT
--

logger:loginfo("Adding first participant p1 ...");
p1 = assert(ivrworx.createcall());
res	 = p1:makecall("sip:conf=conf1@"..hpceaddress..":5060");
assert(res == ivrworx.API_SUCCESS);

logger:loginfo("Configure participant p1 ...");
configure_p1 = [[
<?xml version="1.0" encoding="utf-8"?>
<MediaServerControl version="1.0">
 <request>
   <configure_leg mixmode="full"/>
 </request>
</MediaServerControl>
]];

res	 = p1:sendinfo(configure_p1,"mediaservercontrol+xml");
assert(res == ivrworx.API_SUCCESS);



--
-- PLAY PROMPT
--
play1_req = 
[[
<?xml version="1.0" encoding="utf-8"?>
	   <MediaServerControl version="1.0">
	     <request>
	       <play>
	         <prompt baseurl="http://192.168.100.232/sounds/">
	           <audio url="welcome.wav"/>
	         </prompt>
	       </play>
	     </request>
</MediaServerControl>
]];

res = ccl:sendrawrequest(play1_req);
assert(res == ivrworx.API_SUCCESS);

ivrworx.sleep(5000);




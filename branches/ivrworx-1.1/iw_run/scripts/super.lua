--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--

logger = assert(ivrworx.LOGGER)
conf   = assert(ivrworx.CONF)

logger:loginfo("*** Welcome to super example ! Please change the script to suite your environment! ***");

--[[

--
-- Example application
--

--
-- Make call
-- 
call = assert(ivrworx.createcall());

res	 = call:makecall("sip:1234@10.0.0.2:5060");
assert(res == ivrworx.API_SUCCESS);

--
-- Print agreed codec
-- 
logger:loginfo("codec:"..call:mediaformat());

res = call:play("sounds\\test\\your-callerid-is.wav",true,false);
assert(res == ivrworx.API_SUCCESS);

--
-- You must have MRCP functionality enabled for this
-- to work.
--
res = call:speak("1234",true);
assert(res == ivrworx.API_SUCCESS);

--
-- Play RTSP stream
--
res = call:rtspsetup("rtsp://10.0.0.2/grace.wav");
assert(res == ivrworx.API_SUCCESS);
res = call:rtspplay(0,0,1);
assert(res == ivrworx.API_SUCCESS);
ivrworx.sleep(180000)

--]]

logger:loginfo("super *** Good Bye from super script! ***");

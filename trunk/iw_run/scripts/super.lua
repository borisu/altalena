--
-- Super script runs on application startup
-- You may use any ivrworx api apart from answer
--

logger = assert(ivrworx.LOGGER)
conf   = assert(ivrworx.CONF)

logger:loginfo("*** Welcome to super example ! Please change the script to suite your environment! ***");

--
-- Example application
--

call = assert(ivrworx.createcall());

res	 = call:makecall("sip:1234@10.0.0.1:5060");
assert(res == ivrworx.API_SUCCESS);

res = call:play("sounds\\test\\your-callerid-is.wav",true,false);
assert(res == ivrworx.API_SUCCESS);



--
-- You must have MRCP functionality enabled for this
-- to work.
--
res = call:speak("1234",true);
assert(res == ivrworx.API_SUCCESS);


logger:loginfo("super *** Good Bye! ***");

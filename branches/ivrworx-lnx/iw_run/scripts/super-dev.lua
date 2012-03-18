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

--
-- Make call
--


 
call = assert(ivrworx.createcall());

t={}

t[1] = "1"
t[2] = 1
t[3] = call

ivrworx.spawn("foo.lua",t);

if (true) then return end;


ivrworx.sleep(2000);

computername = os.getenv("COMPUTERNAME");

--
--
--
res	 = call:makecall("sip:1234@"..computername..":5060");
assert(res == ivrworx.API_SUCCESS);

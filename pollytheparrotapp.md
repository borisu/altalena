This is simple IVR application written in ivrworx. Its purpose to server some kind of telephone notebook. Each user has list of key - memo pairs. After simple authentication process, user is asked to enter the key to memo and the application reads it to user.

Application assumes MRCP server existence so you have to have and configure one. Download pollyaddons.zip and extract it in ivrworx root directory. Db contains only one user (user:0000, password 0000), and one cookie (8472)

```
require "ivrworx"
require "luasql.sqlite3"

iw=assert(ivrworx)


function handle_call(caller)
	--
	-- Allocate caller rtp connection.
	--
	local r1, res;


	r1 = assert(rtpproxy:new(), "rtpproxy:new");

	res = r1:allocate{sdp=caller:remoteoffer()};
	assert(res  == iw.API_SUCCESS, "rtpconn:allocate");

	--
	-- Answer call.
	--
	res = caller:answer{sdp=r1:localoffer()};
	assert(res  == iw.API_SUCCESS, "caller:answer");

	--
	-- Allocate mrcp and rtp connection.
	-- Use caller-s local offer temporarily, to set
	-- correct codec set.
	--
	local r2 = assert(rtpproxy:new());
	res = r2:allocate{sdp=r1:localoffer()};
	assert(res  == iw.API_SUCCESS, "r2:allocate");


	--
	-- Allocate MRCP session
	--
	local synth = assert(mrcpsession:new());
	res = synth:allocate{resource=iw.SYNTHESIZER, sdp=r2:localoffer()};
	assert(res  == iw.API_SUCCESS, "synth:allocate");

	--
	-- Modify rtp connection according to MRCP sdp.
	--
	res = r2:modify{sdp=synth:remoteoffer{resource=iw.SYNTHESIZER}};
	assert(res  == iw.API_SUCCESS, "r2:modify");


	res = r1:bridge{other=r2 , duplex="full"}
	l:loginfo("bridge:" ..res);
	assert(res == iw.API_SUCCESS, "r2:bridge");



	--
	-- Gather user id
	--

	local f_greeting, f_noresponse, f_stop, f_fail;
	f_greeting   = function () synth:speak{sentence="Welcome to Polly Application. Please enter your four digits user ID."} end;
	f_noresponse = function () synth:speak{sentence="I have not received your response. Please enter your four digits user ID."} end;
	f_stop       = function () synth:stopspeak() end;
	f_fail 		 = function () synth:speak{sentence="Thanks for using Polly application. Good Bye!", sync=true}; end;

	local input, username, pin, cookieid;
	username = iw.dtmfmenu(f_greeting,f_noresponse,f_stop,f_fail, 10, 30, r1, caller, "%d%d%d%d", true);
	if (username == nil) then
		caller:hangup();
		synth:teardown();
		return;
	end;


	--
	-- Gather secret code
	--
	f_greeting   = function () synth:speak{sentence="Please enter your four digits secret code."} end;
	f_noresponse = function () synth:speak{sentence="I have not received your response. Please enter your four digits secret code."} end;

	pin = iw.dtmfmenu(f_greeting, f_noresponse, f_stop, f_fail, 10 , 30, r1, caller, "%d%d%d%d", true);
	if (pin == nil) then
		synth:teardown();
		caller:hangup();
		return;
	end;

	---
	--- Match user name and password
	---
	local query, cur, row;
	query = "SELECT password from users where id like '"..username.."'";
	l:logdebug(query);

	cur = conn:execute(query)
	if (cur == null) then
		 synth:speak{sentence="Cannot match username or secret code. Please redial.", sync=true}
		 synth:teardown();
		 caller:hangup();
		 return;
	end

	row = cur:fetch ({}, "a")
	if (row == null or row.password ~= pin) then
		 synth:speak{sentence="Secret code is incorrect. Please redial.", sync=true}
		 synth:teardown();
		 caller:hangup();
		 return;
	end

	--
	--  Get the cookie
	--
	f_greeting   = function () synth:speak{sentence="Please enter cookie ID. Then press pound key."} end;
	f_noresponse = function () synth:speak{sentence="I have not received your response. Please enter cookie ID. Then press pound key."} end;
	cookieid = iw.dtmfmenu(f_greeting, f_noresponse, f_stop, f_fail, 10 , 30, r1, caller, "%d*#", true, 20);
	if (cookieid == nil) then
		synth:teardown();
		caller:hangup();
		return;
	end;

	cookieid, occurencies = string.gsub(cookieid, "#", "");

	query = "SELECT value from cookies where id like '"..cookieid.."' AND userid LIKE '"..username.."'";
	l:logdebug(query);

	cur = conn:execute(query)
	row = cur:fetch ({}, "a")
	if (row == null) then
	 synth:speak{sentence="Couldn't find cookie. Please redial.", sync=true}
	 synth:teardown();
	 caller:hangup();
	 return;
	end

	local cookievalue = row.value;
	synth:speak{sentence="Your cookie is : " .. cookievalue, sync=true}

	synth:teardown();
	caller:hangup();

end


--
-- Initate database connection.
--
env = assert(luasql.sqlite3())
l	= assert(iw.LOGGER, "assert:iw.LOGGER")


l:loginfo("== Polly The Parrot == ");

conn = assert(env:connect("polly.sqlite"), "env:connect");

--
-- Wait for someone to call.
--
while (true) do

	local caller, res;

	caller = assert(sipcall:new(), "sipcall:new");
	
	repeat

	 res = caller:accept{service="sip,resip", timeout=10};
	 l:loginfo("caller:accept res:"..res);

	 assert((res == iw.API_TIMEOUT) or (res == iw.API_SUCCESS), "error accepting calls");

	 if (res == iw.API_TIMEOUT) then
	  l:loginfo("Timeout waiting for call.");
	 end

	until (res ~= iw.API_TIMEOUT)



	handle_call(caller);

end
```
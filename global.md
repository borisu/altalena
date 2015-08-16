

# Introduction #

global api

## `.sleep(timeout)` ##

sleeps timeout milliseconds. It is important to use this API instead of os sleep, as it does not block active objects events.

```
require "ivrworx"

local l = assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== SCRIPT START ===");

iw.sleep(5000)

l:loginfo("=== SCRIPT END ===");

```

## `.dtmfmenu()` ##

wait for events on some object. See example of IVR which playes prompt and plays back caller's dtmfs

iw.dtmfmenu(f\_greeting, f\_noresponse, f\_stop , f\_fail, noinputtimeout, maxtimeout, rtpsrc, signalingsrc, pattern, cleandtmfbuffer, maxdigits)

| parameter  | note |
|:-----------|:-----|
| f\_greeting| function for greeting |
| f\_noresponse| function for no response |
| f\_stop    | function upon input start  |
| f\_fail    | function upon input failure |
| noinputtimeout|no input timeout in seconds  |
| maxtimeout |maximum timeout for menu  |
| rtpsrc     |rtp proxy  |
| signalingsrc|sip call  |
| pattern    |lua regex pattern to look for  |
| cleandtmfbuffer|boolean - indicates whether buffer should be cleaned in the end  |
| maxdigits  |maxdigits to look for |

for complete example see polly the parrot application

```

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
```
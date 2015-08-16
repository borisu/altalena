

rtsp session


Currently RTSP stream may be sent only to source of RTSP request. Use rtpproxy to overcome the limtation

## `:new(...)` ##
_ctor_
creates rtspsession object

## `:setup(...)` ##
sends SETUP command

| **named parameter** | **description** |
|:--------------------|:----------------|
| url                 | rtsp url        |
| sdp                 | client side offer  |


## `:play(...)` ##
sends PLAY command

| **named parameter** | **description** |
|:--------------------|:----------------|
| starttime           |start time RTSP paramater|
| duration            | duration RTSP parameter |
| scale               | scale RTSP parameter |


## `:pause(...)` ##
sends PAUSE command


## `:teardown(...)` ##
sends TEARDOWN command

Example. Calling a destination and playing it a file.
###  ###
```
require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")
local res;


--
--  RTSP setup request is sending client ports
-- however it will always send stream to ip or
-- request oroginator that is why we doing trick
-- with rtp proxy.
--

--
-- dummy offer used to set rtp connection codec
--
dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 0.0.0.0
t=0 0
m=audio 0 RTP/AVP 8
a=rtpmap:8 PCMA/8000
]]

--
-- allocate caller proxy connection
--
r1 = rtpproxy:new();
res = r1:allocate{sdp=dummyoffer};
if (res~=iw.API_SUCCESS) then return end;

--
-- allocate proxy connection
--

r2 = rtpproxy:new();
r2:allocate{sdp=dummyoffer};
if (res~=iw.API_SUCCESS) then return end;

--
-- setup call to the caller
--
caller = sipcall:new();
res = caller:makecall{dest="sip:24001@192.168.150.3",sdp=r1:localoffer()}
success(res, "caller:makecall")
res = r1:modify{sdp=caller:remoteoffer()}
if (res~=iw.API_SUCCESS) then return end;

--
-- half duplex bridge
--
res = r2:bridge{other=r1}
if (res~=iw.API_SUCCESS) then return end;


r = rtspsession:new();
res = r:setup{url="rtsp://192.168.100.198/IvrScript.wav", sdp = r2:localoffer()};
if (res~=iw.API_SUCCESS) then return end;

res = r:play();
if (res~=iw.API_SUCCESS) then return end;

caller:waitforhangup()

```


rtpproxy session - brdiges RTP traffic. Also detects rfc2833 dtmf see waitforevent in global example for more info.


## `:new(...)` ##
_ctor_
creates rtpproxyobject

## `:allocate(...)` ##
allocates rtp connection

| **named parameter** | **description** |
|:--------------------|:----------------|
| sdp                 | remote side offer (this is used to build local offer) |


## `:modify(...)` ##
modify rtp destination of the connection

| **named parameter** | **description** |
|:--------------------|:----------------|
| sdp                 | remote side offer (this is used to build local offer) |


## `:bridge(...)` ##
bridges the rtp streams

| **named parameter** | **description** |
|:--------------------|:----------------|
| other               | other connection object to half duplex |
| duplex (1.0.10)     | half | full     |

```
require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")


dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 0.0.0.0
t=0 0
m=audio 0 RTP/AVP 0
a=rtpmap:0 PCMU/8000
]]

--
-- set up first media call
--
r1 = rtpproxy:new();
r1:allocate{sdp=dummyoffer};
caller1 = sipcall:new();
caller1:makecall{dest="sip:24001@192.168.100.164",sdp=r1:localoffer()} 
r1:modify{sdp=caller1:remoteoffer()}

--
-- set up second media call
--
r2 = rtpproxy:new()
r2:allocate{sdp=dummyoffer};
caller2 = sipcall:new()
caller2:makecall{dest="sip:24002@192.168.150.144",sdp=r2:localoffer()} 
r2:modify{sdp=caller2:remoteoffer()}

--
-- half duplex the calls
--
r1:bridge{other=r2}

--
-- full duplex the calls
--
-- r1:bridge{other=r2 , duplex="full"}

caller1:waitforhangup();
caller2:hangup();


```

## `:teardown(...)` ##
deallocates the connection


## `:dtmfbuffer(...)` (1.0.10) ##
returns dtmf buffer

## `:cleanbuffer(...)` ##
cleans dtmf buffer

## `:waitfordtmf(...)`  (1.0.10) ##

| **named parameter** | **description** |
|:--------------------|:----------------|
| timeout             | timeout         |

return result and rfcd2833 dtmf
```
require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("Hello World!");

require "ivrworx"

local l = assert(iw.LOGGER, "assert:iw.LOGGER")


dummyoffer=[[
v=0
o=alice 2890844526 2890844526 IN IP4 0.0.0.0
s=
c=IN IP4 0.0.0.0
t=0 0
m=audio 0 RTP/AVP 0 101
a=rtpmap:0 PCMU/8000
a=rtpmap:101 telephone-event/8000
]]

--
-- set up first media call
--
r1 = rtpproxy:new();
r1:allocate{sdp=dummyoffer};
caller1 = sipcall:new();
caller1:makecall{dest="sip:24001@192.168.150.3",sdp=r1:localoffer()} 
r1:modify{sdp=caller1:remoteoffer()}

res, dtmf = r1:waitfordtmf{timeout=1500}
l:loginfo("dtmf:"..dtmf);
caller1:hangup();

```
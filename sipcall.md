

Encapsulates SIP call functionality

## `:new(...)` ##
_ctor_
creates sipcall object

## `:makecall(...)` ##

| **named parameter** | **description** |
|:--------------------|:----------------|
| dest                | destination     |
| timeout             | client side timeout |
| sdp                 | sdp string to offer (may be used instead of offer/type)|
| offer               | offer           |
| type                | type type       |
| H\_XXX              | XXX header will be added to the SIP message |
| username (1.0.9)    | authentication username  |
| password (1.0.9)    | authentication password  |
| realm  (1.0.9)      | authentication realm |

Authentication data will be re-used if registration was done before on the same object (see register for example).

### example 1 - Make call (empty INVITE) ###
Makes call to the destination, the INVITE sent is empty the call should be answered lately.

```
local caller = sipcall:new()
caller:makecall{dest="sip:24001@192.168.150.3", timeout=15}

localoffer = [[v=0
o=alice 2890844526 2890844526 IN IP4 host.ivrworx.com
s=
c=IN IP4 192.168.150.3
t=0 0
m=audio 49170 RTP/AVP 0
a=rtpmap:0 PCMU/8000

]]

caller:answer{sdp=localoffer}

```


### example 2 - B2B proxy ###
```
local leg1 = sipcall:new()
local leg2 = sipcall:new()

leg1:makecall{dest="sip:24001@192.168.150.3", timeout=15}
leg2:makecall{dest="sip:6095@192.168.150.3", sdp=leg1:remoteoffer(), timeout=15}

leg1:answer{sdp=leg2:remoteoffer()}

```

## `:remoteoffer()` ##

returns raw remote offer

## `:localoffer()` ##

returns raw local offer

## `:waitfordtmf(...)` ##

| **named parameter** | **description** |
|:--------------------|:----------------|
| timeout             | timeout to wait for next DTMF/INFO signal |


## `:gatherdigits(...)` ##

wrapper method
| **named parameter** | **description** |
|:--------------------|:----------------|
| timeout             | timeout to wait for next DTMF/INFO signal |
| pattern             | lua pattern to match for all gathered digits |


### example 1 - Gathering digits ###
```
local caller = sipcall:new()


localoffer = [[v=0
o=ivrworx 123456 654321 IN IP4 10.0.0.1
s=A conversation
c=IN IP4 10.0.0.1
t=0 0
m=audio 6000 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000/1
a=fmtp:101 0-11

]]


caller:makecall{dest="sip:24001@192.168.150.144", timeout=15, sdp=localoffer}
res, dtmf = caller:gatherdigits{pattern="%d#", timeout=15}
l:loginfo(res.."=res".."signal="..dtmf)
```

## `:dtmfbuffer(...)` (1.0.10) ##

return dtmf/INFO buffer

## `:cleandtmfbuffer(...)` (1.0.10) ##

cleans dtmf/INFO buffer

## `:ani(...)` ##

return ANI of the call (user part of From: header)

## `:dnis(...)` ##

return DNIS of the call (user part of To: header)

## `:hangup(...)` ##

Hangs up the call

## `:waitforhangup(...)` ##

Waits for remote party to hang

## `:blindxfer(...)` ##
Xfers the call (REFER)
| **named parameter** | **description** |
|:--------------------|:----------------|
| dest                | xfer destination |

### example 1 - Blind xfer ###
```
local caller = sipcall:new()


localoffer = [[v=0
o=ivrworx 123456 654321 IN IP4 10.0.0.1
s=A conversation
c=IN IP4 10.0.0.1
t=0 0
m=audio 6000 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000/1
a=fmtp:101 0-11

]]


caller:makecall{dest="sip:24001@192.168.150.142", timeout=15, sdp=localoffer}
caller:blindxfer{dest="sip:24001@192.168.150.144"}
```

## `:sendinfo(...)` ##
sends INFO
| **named parameter** | **description** |
|:--------------------|:----------------|
| offer               | body of INFO    |
| type                | type of body INFO |
| async               | if true waits and returns response |

## `:accept(...)` ##
accepts incoming SIP call

| **named parameter** | **description** |
|:--------------------|:----------------|
| service             | stack identifier |
| timeout             | time to waitin for incoming call |

### example : Accepting the call ###

```
local caller  = sipcall:new()



localoffer = [[v=0
o=ivrworx 123456 654321 IN IP4 10.0.0.1
s=A conversation
c=IN IP4 10.0.0.1
t=0 0
m=audio 6000 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000/1
a=fmtp:101 0-11

]]


caller:accept{service="sip,resip", timeout=600}
caller:answer{sdp=localoffer} 
```

## `:startregister(...)` ##

starts registration

| **named parameter** | **description** |
|:--------------------|:----------------|
| registrar           | registrar URI and To: example "sip:1234@example.com"|
| timeout             | client side timeout |
| username            | user name       |
| password            | password        |
| realm               | realm           |

```
c1 = sipcall:new();

res = c1:startregister{registrar="sip:1000@192.168.100.241:5060", 
			username="1000", 
			password="1234",
			realm="192.168.100.241",
			timeout=15};

l:loginfo("c1:startregister res = " .. res);
if (res~=iw.API_SUCCESS) then return end;

res = c1:makecall{dest="sip:907723154123@192.168.100.241"}  
l:loginfo("c1:makecall res = " .. res);
if (res~=iw.API_SUCCESS) then return end;



iw.sleep(10000)
```

## `:unregister(...)` ##

stops registration

## `:reoffer(...)` ##

sends reoffer (re-invite) request

| **named parameter** | **description** |
|:--------------------|:----------------|
| timeout             | client side timeout |
| sdp                 | sdp string to offer (may be used instead of offer/type)|
| offer               | offer           |
| type                | type type       |
| H\_XXX              | XXX header will be added to the SIP message |


example
```
require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("Hello World!");

c = sipcall:new();


localoffer = [[v=0
o=ivrworx 123456 654321 IN IP4 10.0.0.1
s=A conversation
c=IN IP4 10.0.0.1
t=0 0
m=audio 6000 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:8 PCMA/8000/1
a=rtpmap:101 telephone-event/8000/1
a=fmtp:101 0-11

]]


c:makecall{dest="sip:24001@192.168.150.3", timeout=15, sdp=localoffer}
c:reoffer{timeout=15, sdp=localoffer}
c:hangup();
```

## `:subscribe(...)/:waitfornotify()` (1.0.9) ##

starts subscription

| **named parameter** | **description** |
|:--------------------|:----------------|
| server              | Request-URI and To: example "sip:1234@example.com"|
| package             | events package  |
| interval            | subscription interval |
| refresh             | subscription refresh |
| timeout             | client side timeout |
| username            | user name       |
| password            | password        |
| realm               | realm           |


```
require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== START ===");

--
-- set registration session to receive events upon
--
r= sipcall:new();			
r:startregister{registrar="sip:user1@192.168.100.242:5080", 
                        username="user1", 
                        password="1234",
                        realm="192.168.100.242",
                        timeout=15};

s = sipcall:new()
s:subscribe{
	server="sip:user1@192.168.100.242:5080",
	package="presence", 
	user="user1",
	password="1234",
	realm="192.168.100.242",
	interval=60,
	refresh=180, 
	timeout=300};
	
x,y,z = s:waitfornotify{timeout=15}

l:loginfo(z);

l:loginfo("=== END ===");						
```
# Introduction #

Bridge b2b calls into free switch conference. Should work with out-of-box freeswitch configuration

Change your environment settings accordingly:-

|GW | 192.168.150.3 |
|:--|:--------------|
|extension bridged into conference| 24001,24012   |
|freeswitch extensions used for b2b call | 1001,1002,1003 |
|freeswitch ip | 192.168.100.241 |
|freeswitch conference extensions | 3001          |



```
require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")
local res;



--
-- add first call to the conference
--
confleg1 = sipcall:new();
res = confleg1:startregister{registrar="sip:1000@192.168.100.241:5060",
                        username="1000",
                        password="1234",
                        realm="192.168.100.241",
                        timeout=15};
if (res~=iw.API_SUCCESS) then return end;


outleg1 = sipcall:new()
res = outleg1:makecall{dest="sip:24001@192.168.150.3", timeout=15}
if (res~=iw.API_SUCCESS) then return end;

res = confleg1:makecall{dest="sip:3001@192.168.100.241", sdp=outleg1:remoteoffer(), timeout=15}
if (res~=iw.API_SUCCESS) then return end;

outleg1:answer{sdp=confleg1:remoteoffer()}
if (res~=iw.API_SUCCESS) then return end;



--
-- add second call to conference
--
confleg2 = sipcall:new();
res = confleg2:startregister{registrar="sip:1001@192.168.100.241:5060",
                        username="1001",
                        password="1234",
                        realm="192.168.100.241",
                        timeout=15};
if (res~=iw.API_SUCCESS) then return end;

outleg2 = sipcall:new()
res = outleg2:makecall{dest="sip:24012@192.168.150.3", timeout=15}
if (res~=iw.API_SUCCESS) then return end;

res = confleg2:makecall{dest="sip:3001@192.168.100.241", sdp=outleg2:remoteoffer(), timeout=15}
if (res~=iw.API_SUCCESS) then return end;

outleg2:answer{sdp=confleg2:remoteoffer()}
if (res~=iw.API_SUCCESS) then return end;


---
--- add streamer to conference
---
confleg3 = sipcall:new();
res = confleg3:startregister{registrar="sip:1002@192.168.100.241:5060",
                        username="1002",
                        password="1234",
                        realm="192.168.100.241",
                        timeout=15};
if (res~=iw.API_SUCCESS) then return end;

s = streamer:new();
s:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE}


res = confleg3:makecall{dest="sip:3001@192.168.100.241", sdp=s:localoffer(), timeout=15}
if (res~=iw.API_SUCCESS) then return end;


s:modify{sdp=confleg3:remoteoffer()}
s:play{file="greeting.wav", sync=true}



---
---
---
confleg1:hangup();
confleg2:hangup();
outleg1:hangup();
outleg2:hangup();











```
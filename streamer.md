Simple built-in streamer

## `:new(...)` ##
_ctor_
creates streamer object

## `:allocate(...)` ##

| **named parameter** | **description** |
|:--------------------|:----------------|
| dest                | destination     |
| timeout             | client side timeout |
| sdp                 | sdp string to offer (may be used insted of offer/type)|
| rcv                 | type of receiving device (see table below for possible values)|
| snd                 | type of device generating audio stream (see table below for possible values)|

| **sending device id** | **description** |
|:----------------------|:----------------|
| SND\_DEVICE\_TYPE\_FILE | file            |
| SND\_DEVICE\_TYPE\_SND\_CARD\_MIC | default line-in |


| **receiving device id** | **description** |
|:------------------------|:----------------|
| RCV\_DEVICE\_NONE       | none            |
| RCV\_DEVICE\_FILE\_REC\_ID | wav file        |
| RCV\_DEVICE\_WINSND\_WRITE | default output  |


## `:play(...)` ##

WAV file MUST be in the same sampling rate (PCM) as RTP stream. In case of error you might see the following log line:
> clock rates of payload and wav file are different pt:8000 file:44100 imsh:7000000


| **named parameter** | **description** |
|:--------------------|:----------------|
| file                | flename to play (if open with SND\_DEVICE\_TYPE\_FILE)|
| loop                | the file will be played in loop|
| sync                | if true the function will exit upon eof (cannot be used with loop = true|

Relative filepaths are taken from "m2ims/sounds\_dir" configuration value.

## `:stop()` ##

stops playing the file

## `:remoteoffer()` ##

returns raw remote offer

## `:localoffer()` ##

returns raw local offer

## `:modify(...)` ##

| **named parameter** | **description** |
|:--------------------|:----------------|
| sdp                 | remote offer (modifies destination of the streaming)|

### example 1 ###
Makes call to the destination, the INVITE sent is empty the call should be answered lately.

```
require "ivrworx"

s = streamer:new();
s:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE}

caller = sipcall:new();
caller:makecall{dest="sip:24001@192.168.150.3", timeout=60,sdp=s:localoffer()}


s:modify{sdp=caller:remoteoffer()}
s:play{file="C:\\sounds\\GREETING.wav", loop=true}

caller:waitforhangup();

```
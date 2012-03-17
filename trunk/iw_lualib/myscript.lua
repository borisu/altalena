require "ivrworx"

s = streamer:new();
s:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE}

s1 = streamer:new();
s1:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE}

caller = sipcall:new();
caller:makecall{dest="sip:24001@10.116.100.79", timeout=60,sdp=s:localoffer()}


s:modify{sdp=caller:remoteoffer()}
s:play{file="C:\\sounds\\GREETING.wav", loop=true}

s1:modify{sdp=caller:remoteoffer()}
s1:play{file="C:\\sounds\\GREETING.wav", loop=true}


caller:waitforhangup();
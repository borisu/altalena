require "ivrworx"

s = streamer:new();
s:allocate{rcv=iw.RCV_DEVICE_NONE, snd=iw.SND_DEVICE_TYPE_FILE}

caller = sipcall:new();
caller:makecall{dest="sip:6050@10.116.24.21", timeout=60,sdp=s:localoffer()}


s:modify{sdp=caller:remoteoffer()}
s:play{file="C:\\sounds\\GREETING.wav", loop=true}

caller:waitforhangup();

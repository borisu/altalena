module("ivrworx",package.seeall)

function answer()
	return this.answer(this);
end

function hangup()
	return this.hangup(this);
end

function wait(timeout)
	return this.wait(this,timeout);
end

function play(file,sync,loop)
	return this.play(this,file,sync,loop);
end

function send_dtmf(dtmf)
	return this.send_dtmf(this,dtmf);
end

function wait_for_dtmf(timeout)
	return this.wait_for_dtmf(this,timeout);
end
		
		
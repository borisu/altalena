module("ivrworx",package.seeall)

function run(f)
	return this.run(this,f);
end

function answer()
	return this.answer(this);
end

function hangup()
	return this.hangup(this);
end

function wait(timeout)
	return this.wait(this,timeout);
end

function wait_till_hangup()
	return this.wait_till_hangup(this);
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

function blind_xfer(destination)
	return this.blind_xfer(this,destination);
end

function make_call(destination)
	return this.make_call(this,destination);
end

LOG_LEVEL_OFF = 0;
LOG_LEVEL_CRITICAL = 1;
LOG_LEVEL_WARN = 2;
LOG_LEVEL_INFO = 3;
LOG_LEVEL_DEBUG = 4;
LOG_LEVEL_TRACE = 5;

function iw_log(level, str)
	return this.iw_log(this, level, str);
end

function logcrt(str)
	return iw_log(LOG_LEVEL_CRITICAL,str);
end

function logwrn(str)
	return iw_log(LOG_LEVEL_WARN,str);
end

function loginf(str)
	return iw_log(LOG_LEVEL_INFO,str);
end

function logdbg(str)
	return iw_log(LOG_LEVEL_DEBUG,str);
end

function logtrc(str)
	return iw_log(LOG_LEVEL_TRACE,str);
end
		
		
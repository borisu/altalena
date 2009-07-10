---
--- Super script is run upon system startup
---
require "ivrworx"

ivrworx.loginf("Super script started");

res,handle = ivrworx.make_call("sip:1234@10.0.0.2:5062");

if (res ~= 0) then
	ivrworx.loginf("Error making call res:" .. res)
else
	ivrworx.loginf("Sucess, call handle:" .. handle);
end





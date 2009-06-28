---
--- Super script is run upon system startup
---
require "ivrworx"

ivrworx.logdbg("script> super script started");

a = 1
ivrworx.run(function() a = 4 end )
ivrworx.loginf("script>"..a)

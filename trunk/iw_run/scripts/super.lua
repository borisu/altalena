---
--- Super script is run upon system startup
---
require "ivrworx"
require "ivrworx_lanes"

local il = ivrworx_lanes

f= ivrworx_lanes.gen( function(n) return 2*n end )

a= f(1)
b= f(2)

ivrworx.loginf( "script>"..a[1])
ivrworx.loginf( "script>"..b[1])
require "iw_lualib"

local shutdownproxy = newproxy(true)  -- create proxy object with new metatable
assert(type(shutdownproxy) == 'userdata')
getmetatable(shutdownproxy).__gc = function() 
		if ((ivrworx ~= nil) and (ivrworx.LOGGER ~= nil)) then
			ivrworx.LOGGER:loginfo("ivrworx: shutting down ..."); 
			ivrworx.close(); 
		else
			print("error loading ivrworx");
		end
	end





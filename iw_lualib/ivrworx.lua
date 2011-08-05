require "iw_lualib"
--require "sdphelper"

iw=assert(ivrworx)

--
-- create ivrworx destructor
--
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

--
-- assrert function has returned iw.API_SUCCESS
--
function success(x,y)
	if (x==nil or type(x) ~= "number" or x ~= iw.API_SUCCESS) then
		print(y);
		error(y);
	end;
	return x;
end

local function getopt(arg, options)
    local opt, optind = {}, 1
    local waiting

    for _,v in ipairs(arg) do
        if waiting then
            -- short option waiting for a value
            opt[waiting] = v
            optind = optind + 1
            waiting = nil
        elseif v == "-" then
            break
        elseif v:sub(1, 1) == "-" then
            optind = optind + 1
            if v == "--" then
                break
            elseif v:sub(1, 2) == "--" then
                -- long option
                local x = v:find("=", 1, true)
                if x then
                    opt[v:sub(3, x-1)] = v:sub(x+1)
                else
                    opt[v:sub(3)] = true
                end
            else
                -- short option
                local j, l = 2, #v
                while (j <= l) do
                    local t = v:sub(j, j)
                    local x = options:find(t, 1, true)
                    if t == ":" then
                        io.stderr:write(arg[0],": invalid option --'", t, "'\n")
                        opt["?"] = true
                    elseif x then
                        if options:sub(x+1, x+1) == ":" then
                            local w = v:sub(j+1)
                            if #w > 0 then
                                opt[t] = w
                                j = l
                            else
                                waiting = t
                            end
                        else
                            opt[t] = true
                        end
                    else
                        io.stderr:write(arg[0],": invalid option --'", t, "'\n")
                        opt["?"] = true
                    end
                    j = j + 1
                end
            end
        else
            break
        end
    end

    if waiting ~= nil then
        io.stderr:write(arg[0],": option requires an argument -- '",waiting,"'\n")
        opt[":"] = true
    end

    return opt, optind
end


args = getopt(arg,"conf")
if (args.conf ~= nil) then
	conffile = args.conf
else
    conffile = "conf.json"
end


success(iw.init(conffile), "assert:iw.init")

function sipcall:gatherdigits(arg)
	if (arg == nil or arg.pattern == nil) then
		return iw.API_WRONG_PARAMETER, ""
	end

	local digits = "";
	local res = iw.API_SUCCESS
	local pattern = arg.pattern;

	while (string.find(digits, pattern)==nil and res == iw.API_SUCCESS) do
		res, signal = self:waitfordtmf(arg);
		digits = digits..signal;
	end

	return res, digits

end








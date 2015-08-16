

# Introduction #

iw.LOGGER is the object representing current ivrworx logging infrastructure. It is always supplied as global ivrworx member after initialization.

```
require "ivrworx"
iw=ivrworx

--
-- Get ivrworx logger
--
logger= assert(iw.LOGGER)

logger:loginfo("This is ivrworx message");

```



## `:loginfo(string)` ##

logs INFO levek message

## `:logwarn(string)` ##

logs WARN level message

## `:logcrit(string)` ##

logs CRIT level message

## `:logdebug(string)` ##

logs DEBUG level message
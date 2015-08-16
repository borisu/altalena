

# Introduction #

ConfBridge is the object representing current ivrworx configuration.

it always supplied as global ivrworx member after initialization.

```
require "ivrworx"


conf = assert(iw.CONF)

```

You may access underlying configuration in a hierarchical manner, by using slashes notations. This is only works for object JSON type as parent . For example for following configuration file:-
```
{
	
    "sqlite" : {
	"uri" : "sql,sqlite"
    }
}
```

you may access the property by
**`ivrworx.CONF:getstring("sqlite/uri")`**


## `:getstring(string)` ##

gets string value from configuration object

## `:getboolean(string)` ##

gets boolean value from configuration object

## `:getint(string)` ##

gets  int value from configuration object

## `:getarray(string)` ##

gets  int value from configuration object
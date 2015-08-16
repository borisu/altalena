

Selector is used to wait for events from multiple sources.

## `:new(...)` ##
_ctor_
creates selector object

## `:select (...)` ##
selects from list of actors. (List of actors is built only upon first call).

| parameter | notes |
|:----------|:------|
| actors    | list of event sources |
| timeout   | timeout |

see `iw.dtmfmenu` for example

```
--
-- start collecting digits
--
s = selector:new();
res,i = s:select{actors={rtpsrc, signalingsrc}, timeout=0};

while (true) do
	--
	-- wait for hangup or dtmf
	--
	res,i = s:select{timeout=noinputtimeout};
	
end
```
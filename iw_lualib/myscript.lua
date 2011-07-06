require "ivrworx"

local l	= assert(iw.LOGGER, "assert:iw.LOGGER")

l:loginfo("=== START ===");

c = sipcall:new();

l:loginfo("running 5");
iw.waitforevent{actors={c}, timeout=15};
if (index == nil) then index = -1; end;
l:loginfo("5. res:"..res.." index:"..index);	

l:loginfo("running 1");
res, index = iw.waitforevent();
if (index == nil) then index = -1; end;
l:loginfo("1. res:"..res.." index:"..index);	

l:loginfo("running 2");
res, index = iw.waitforevent(c);
if (index == nil) then index = -1; end;
l:loginfo("2. res:"..res.." index:"..index);	

l:loginfo("running 3");
res, index = iw.waitforevent{actors="fff"};
if (index == nil) then index = -1; end;
l:loginfo("3. res:"..res.." index:"..index);	

l:loginfo("running 4");
res, index = iw.waitforevent{actors={}};
if (index == nil) then index = -1; end;
l:loginfo("4. res:"..res.." index:"..index);	



l:loginfo("=== END ===");						














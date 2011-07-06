#pragma once
#include "Luna.h"
#include "LuaObject.h"

namespace ivrworx
{
	class rtspsession :
		public luaobject
	{
	public:
		rtspsession(RtspSessionPtr rtspSession);
		rtspsession(lua_State *L);
		virtual ~rtspsession(void);

		int setup(lua_State *L);
		int play(lua_State *L);
		int pause(lua_State *L);
		int teardown(lua_State *L);
		int remoteoffer(lua_State *L);
		int localoffer(lua_State *L);
		

		static const char className[];
		static Luna<rtspsession>::RegType methods[];

	private:

		RtspSessionPtr _rtspSession;
	};


}


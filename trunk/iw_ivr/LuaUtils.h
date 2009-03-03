#pragma once

namespace ivrworx
{
	class LuaVm
	{
	public:

		LuaVm();

		virtual ~LuaVm();

		ApiErrorCode InitializeVm();

		void DestroyVm();

		static void Panic (lua_State *lua);

	private:

		bool _intialized;

		lua_State *_state;
	};

	class LuaScript
	{
	public:

		LuaScript(LuaVm &vm);

		int RegisterFunction (const char *func_name);

	};

}


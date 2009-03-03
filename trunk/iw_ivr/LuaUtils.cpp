#include "StdAfx.h"
#include "LuaUtils.h"

namespace ivrworx
{
	
	static int printMessage (lua_State *lua)
	{
		assert (lua_isstring (lua,1));

		const char *msg = lua_tostring (lua, 1);

		// get caller
		lua_Debug ar;
		memset (&ar, 0, sizeof(ar));
		lua_getstack (lua, 1, &ar);
		lua_getinfo (lua, "Snl", &ar);

		// debug output
		const char *str = ar.source;
		LogInfo("script: " << msg << " -- at " << str << "(" << ar.currentline << ")");
		return 0;
	}
	
	LuaVm::LuaVm():
	_intialized(false),
	_state(NULL)
	{

	}


	LuaVm::~LuaVm()
	{

	}

	ApiErrorCode 
	LuaVm::InitializeVm()
	{
		
		if (_intialized) 
		{
			LogWarn("Destroying vm due to multiple init");
			DestroyVm ();
		}

		_state = lua_open ();

		if (_state) 
		{
			_intialized = true;

			luaL_openlibs (_state);

			// setup global printing (trace)
			lua_pushcclosure (_state, printMessage, 0);
			lua_setglobal (_state, "trace");

			lua_atpanic (_state, (lua_CFunction) LuaVm::Panic);

		}

		return API_SUCCESS;
	}

	void 
	LuaVm::DestroyVm (void)
	{
		if (_state)
		{
			lua_close (_state);
			_state = NULL;
			_intialized = false;
		}
	}

	

	int
	LuaScript::RegisterFunction(const char *func_name)
	{

	}
	

}


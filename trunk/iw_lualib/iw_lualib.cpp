// iw_lualib.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "LuaRestoreStack.h"
#include "LightweightProcess.h"

using namespace ivrworx;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

extern "C" {
	int LUA_API luaopen_iw_lualib(lua_State *L);
	int LUA_API init(lua_State *L);
	int LUA_API close(lua_State *L);
}


static const luaL_reg testlib[] = 
{
	{"init", init},
	{"close", close},
	{NULL, NULL}
};

static ConfigurationPtr		g_conf;
static LpHandlePair			g_CtxPair;
static RunningContext	   *g_Ctx = NULL;
static HandlePairList		g_procHandlePairs;
static HandlesVector		g_shutdownHandles;
static FactoryPtrList		g_factoriesList;
static ScopedForking	   *g_forking = NULL;
static ivrworx::Context		g_LuaCtx;

int LUA_API close(lua_State *L)
{
	if (g_forking)
	{

		ShutdownModules(g_procHandlePairs,g_conf);
	}


	LocalProcessRegistrar().Instance().UnReliableShutdownAll();

	delete g_forking;
	g_forking = NULL;
	return 0;
}

int LUA_API init(lua_State *L)
{
	
	OSVERSIONINFO osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

// 	if ( osvi.dwMajorVersion  < 6)
// 	{
// 		cerr << ("The system does not meet the requirements.\n");
// 		return 1;
// 	}


	if (g_Ctx != NULL)
	{
		cerr << "init - cannot initialize the library twice" << endl;
		lua_pushnumber(L,API_TIMER_INIT_FAILURE);
		return 1;
	}

	
	int res = 0;
	try
	{
	
		const char* conffile = lua_tostring(L,-1);
		if (conffile == NULL)
		{
			cerr << "init:g_conf file is NULL using conf.json" << endl;
			conffile = "conf.json";
		} else
		{
			cout << "loading " << conffile << "..." << endl;

		}
 
 		ApiErrorCode err_code = API_SUCCESS;
 		g_conf = ConfigurationFactory::CreateJsonConfiguration(conffile,err_code);
 
 		if (IW_FAILURE(err_code))
 		{
			cerr << "init:error reading configuration file:" << conffile << endl;
			lua_pushnumber(L,API_WRONG_PARAMETER);
			return 1;
 		}
 
 
 		if (!InitLog(g_conf))
		{
			cerr << "init:error initiating logging infrastructure:" << endl;
			lua_pushnumber(L,API_WRONG_PARAMETER);
			return 1;
		}

 	
 		// from here you may use generic clean up routine
		LogInfo(">>>>>> IVRWORX (LUA) START <<<<<<");
		Start_CPPCSP();

		DECLARE_NAMED_HANDLE_PAIR(global_ctx_pair);

		g_CtxPair = global_ctx_pair;
		g_Ctx = new RunningContext(g_CtxPair,"LuaScript");

		RegisterContext(g_Ctx);

		
		if (IW_FAILURE(LoadConfiguredModules(
				g_conf,
				g_factoriesList)))
		{
			goto error;
		};

		// this is something THAT IS NOT ADVISED by CSP
		// but no way I can do it any other way
		g_forking = new ScopedForking();
		if (IW_FAILURE(BootModulesSimple(
				g_conf,
				*g_forking,
				g_factoriesList,
				g_procHandlePairs,
				g_shutdownHandles)))
		{
			goto error;
		};

		g_LuaCtx._forking = g_forking;
		g_LuaCtx._conf = g_conf;
		g_Ctx->SetAppData(&g_LuaCtx);

		LuaTable table(L);
		table.Attach("ivrworx");

		if (InitStaticTypes(L,table,&g_LuaCtx) == FALSE)
		{
			goto error;
		}



	
	} 
	catch (exception e)
	{
		cerr << endl << "Exception caught during program execution e:" << e.what() << endl;
		goto error;
	}


	lua_pushnumber(L,API_SUCCESS);
	return 1;

error:
	

	End_CPPCSP();
	LogInfo(">>>>>> IVRWORX END <<<<<<");
	ExitLog();

	lua_pushnumber(L,API_FAILURE);
	return 1;

}


/*
** Open test library
*/
int LUA_API luaopen_iw_lualib(lua_State *L)
{
	luaL_openlib(L, "ivrworx", testlib, 0);
	return 1;
}


#ifdef _MANAGED
#pragma managed(pop)
#endif


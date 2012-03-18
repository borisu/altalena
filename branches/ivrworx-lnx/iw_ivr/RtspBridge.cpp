#include "StdAfx.h"
#include "RtspBridge.h"
#include "BridgeMacros.h"

namespace ivrworx
{
	const char rtspsession::className[] = "rtspsession";
	Luna<rtspsession>::RegType rtspsession::methods[] = {
		method(rtspsession, setup),
		method(rtspsession, play),
		method(rtspsession, pause),
		method(rtspsession, teardown),
		method(rtspsession, remoteoffer),
		method(rtspsession, localoffer),
		{0,0}
	};


	rtspsession::rtspsession(RtspSessionPtr rtspSession):
	_rtspSession(rtspSession)
	{

	}

	rtspsession::rtspsession(lua_State *L)
	{

	}

	rtspsession::~rtspsession(void)
	{
	}

	int 
	rtspsession::setup(lua_State *L)
	{
		if (!_rtspSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		string url;
		GetTableStringParam(L,-1,url,"url");

		AbstractOffer offer;
		offer.type = "application/sdp";
		BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
		if (paramres == FALSE)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}


		ApiErrorCode res = _rtspSession->Setup(url,offer);

		lua_pushnumber (L, res);
		return 1;
		

	}

	int 
	rtspsession::remoteoffer(lua_State *L)
	{
		FUNCTRACKER;
		if (!_rtspSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _rtspSession->RemoteOffer().body.c_str());
		return 1;

	}

	int 
	rtspsession::localoffer(lua_State *L)
	{
		FUNCTRACKER;
		if (!_rtspSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _rtspSession->LocalOffer().body.c_str());
		return 1;

	}

	int 
	rtspsession::play(lua_State *L)
	{
		if (!_rtspSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		double start_time = 0.0;
		GetTableNumberParam(L,-1,&start_time,"starttime",0.0);

		double duration = 0.0;
		GetTableNumberParam(L,-1,&duration,"duration",0.0);

		double scale = 0.0;
		GetTableNumberParam(L,-1,&duration,"scale",0.0);

		ApiErrorCode res = _rtspSession->Play(start_time,duration, scale);

		lua_pushnumber (L, res);
		return 1;
	}

	int 
	rtspsession::pause(lua_State *L)
	{
		if (!_rtspSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		ApiErrorCode res = _rtspSession->Pause();

		lua_pushnumber (L, res);
		return 1;

	}

	int 
	rtspsession::teardown(lua_State *L)
	{
		if (!_rtspSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		ApiErrorCode res = _rtspSession->TearDown();

		lua_pushnumber (L, res);
		return 1;

	}

}

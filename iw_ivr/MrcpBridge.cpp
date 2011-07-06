#include "StdAfx.h"
#include "MrcpBridge.h"
#include "BridgeMacros.h"

namespace ivrworx
{
	const char mrcpsession::className[] = "mrcpsession";
	Luna<mrcpsession>::RegType mrcpsession::methods[] = {
		method(mrcpsession, allocate),
		method(mrcpsession, stopspeak),
		method(mrcpsession, speak),
		method(mrcpsession, teardown),
		method(mrcpsession, localoffer),
		method(mrcpsession, remoteoffer),
		method(mrcpsession, recognize),
		method(mrcpsession, waitforrecogresult),
		{0,0}
	};

	mrcpsession::mrcpsession(MrcpSessionPtr mrcp_session):
	_mrcpSession(mrcp_session)
	{

	}

	mrcpsession::mrcpsession(lua_State *L)
	{

	}

	mrcpsession::~mrcpsession(void)
	{
		
	}

	int 
	mrcpsession::remoteoffer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		MrcpResource resource = RECOGNIZER;
		GetTableEnumParam<MrcpResource>(L,-1,&resource,"resource");


		lua_pushstring(L, _mrcpSession->RemoteOffer(resource).body.c_str());
		return 1;

	}

	int 
	mrcpsession::localoffer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		MrcpResource resource = UKNOWN_MRCP_RESOURCE;
		BOOL paramres = GetTableEnumParam<MrcpResource>(L,-1,&resource,"resource");
		if (paramres == FALSE)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}


		lua_pushstring(L, _mrcpSession->LocalOffer(resource).body.c_str());
		return 1;

	}

	
	int 
	mrcpsession::allocate(lua_State *L)
	{
		if (!_mrcpSession)
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

		
		MrcpResource temp = RECOGNIZER;
		GetTableEnumParam<MrcpResource>(L,-1,&temp,"resource");
		if (paramres == FALSE)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}

		int secs  = 15;
		GetTableNumberParam<int>(L,-1,&secs,"timeout",15);

		ApiErrorCode res  = _mrcpSession->Allocate(temp, offer, Seconds(secs));
		lua_pushnumber (L, res);

		return 1;


	}

	int 
	mrcpsession::waitforrecogresult(lua_State *L)
	{
		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		int secs  = 15;
		GetTableNumberParam<int>(L,-1,&secs,"timeout");

		string answer;

		ApiErrorCode res  = _mrcpSession->WaitForRecogResult(Seconds(secs),answer);
		lua_pushnumber (L, res);

		return 1;

	}

	int 
	mrcpsession::recognize(lua_State *L)
	{
		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		int secs  = 15;
		GetTableNumberParam(L,-1,&secs,"timeout",15);

		string body;
		GetTableStringParam(L,-1,body,"grammar");
		if (body.empty())
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}

		bool sync;
		GetTableBoolParam(L,-1,&sync,"sync");

		MrcpParams p;
		FillTable(L,-1,p);

		string answer;
		ApiErrorCode res  = _mrcpSession->Recognize(p, body,Seconds(secs),sync,answer);

		lua_pushnumber (L, res);
		lua_pushstring(L, answer.c_str());

	
		return 2;

	}

	
	int 
	mrcpsession::stopspeak(lua_State *L)
	{
		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}
		
		ApiErrorCode res  = _mrcpSession->StopSpeak();
		lua_pushnumber (L, res);

		return 1;

	}

	int 
	mrcpsession::speak(lua_State *L)
	{
		FUNCTRACKER;

		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		bool sync = false;
		string sentence;
		GetTableBoolParam(L,-1,&sync,"sync",false);
		GetTableStringParam(L,-1,sentence,"sentence");

		string rawbody;
		GetTableStringParam(L,-1,rawbody,"rawbody");

		if (sentence.empty() && rawbody.empty())
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;
		}

		MrcpParams p;
		FillTable(L,-1,p);


		
		stringstream mrcp_body;
		if (!sentence.empty())
		{
			p["content_type"] = string("application/synthesis+ssml");
			mrcp_body 
				<< "<?xml version=\"1.0\"?>"	 											<< endl 
				<< "<speak version=\"1.0\" xml:lang=\"en-US\" xmlns=\"http://www.w3.org/2001/10/synthesis\">"  << endl
				<< " <p>"																    << endl
				<< "   <s>"<<sentence<<"</s>"												<< endl
				<< " </p>"																	<< endl
				<< "</speak>"																<< endl;	
		} else
		{
			mrcp_body << rawbody;
		}
		


		ApiErrorCode res =_mrcpSession->Speak(p,mrcp_body.str(),sync);

		lua_pushnumber (L, res);
		return 1;

	}

	shared_ptr<ActiveObject> 
	mrcpsession::get_active_object() 
	{
		return dynamic_pointer_cast<ActiveObject>(_mrcpSession);

	}

	int 
	mrcpsession::teardown(lua_State *L)
	{
		return 0;

	}



}


#include "StdAfx.h"
#include "MrcpBridge.h"
#include "BridgeMacros.h"

namespace ivrworx
{
	const char MrcpBridge::className[] = "MrcpBridge";
	Luna<MrcpBridge>::RegType MrcpBridge::methods[] = {
		method(MrcpBridge, allocate),
		method(MrcpBridge, modify),
		method(MrcpBridge, stopspeak),
		method(MrcpBridge, speak),
		method(MrcpBridge, teardown),
		method(MrcpBridge, localcnx),
		method(MrcpBridge, remotecnx),
		method(MrcpBridge, recognize),
		method(MrcpBridge, waitforrecogresult),
		{0,0}
	};

	MrcpBridge::MrcpBridge(MrcpSessionPtr mrcp_session):
	_mrcpSession(mrcp_session)
	{

	}

	MrcpBridge::MrcpBridge(lua_State *L)
	{

	}

	MrcpBridge::~MrcpBridge(void)
	{
		
	}

	int 
	MrcpBridge::remotecnx(lua_State *L)
	{
		FUNCTRACKER;

		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		MrcpResource temp = RECOGNIZER;
		GetTableEnumParam<MrcpResource>(L,-1,&temp,"resource_int");

		throw;

// 		Luna<CnxInfoBridge>::PushObject(L, new CnxInfoBridge(_mrcpSession->RemoteMedia(temp)));
// 		return 1;

	}

	int 
	MrcpBridge::localcnx(lua_State *L)
	{
		FUNCTRACKER;

		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		};

		MrcpResource temp = UKNOWN_MRCP_RESOURCE;
		GetTableEnumParam<MrcpResource>(L,-1,&temp,"resource_int");

		throw;


// 		Luna<CnxInfoBridge>::PushObject(L, new CnxInfoBridge( _mrcpSession->LocalMedia(temp)));
// 		return 1;

	}

	
	int 
	MrcpBridge::allocate(lua_State *L)
	{
// 		if (!_mrcpSession)
// 		{
// 			lua_pushnumber (L, API_WRONG_STATE);
// 			return 1;
// 		}
// 
// 		CnxInfoBridge *cnx_info = NULL;
// 		GetLunaUserData<CnxInfoBridge>(L,-1,&cnx_info,"localcnx_ut");
// 
// 		string format;
// 		GetTableStringParam(L,-1,format,"format_str");
// 		MediaFormat f = MediaFormat::GetMediaFormat(format);
// 
// 		
// 		MrcpResource temp = RECOGNIZER;
// 		GetTableEnumParam<MrcpResource>(L,-1,&temp,"resource_int");
// 
// 		int secs  = 15;
// 		GetTableNumberParam<int>(L,-1,&secs,"timeout");
// 
// // 		ApiErrorCode res  = _mrcpSession->Allocate(temp, cnx_info->_cnxInfo, f, Seconds(secs));
// // 		lua_pushnumber (L, res);

		return 1;


	}

	int 
	MrcpBridge::waitforrecogresult(lua_State *L)
	{
		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		int secs  = 15;
		GetTableNumberParam<int>(L,-1,&secs,"timeout");

		ApiErrorCode res  = _mrcpSession->WaitForRecogResult(Seconds(secs));
		lua_pushnumber (L, res);

		return 1;

	}

	int 
	MrcpBridge::recognize(lua_State *L)
	{
		if (!_mrcpSession)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		int secs  = 15;
		GetTableNumberParam(L,-1,&secs,"timeout");

		string body;
		GetTableStringParam(L,-1,body,"body");

		bool sync;
		GetTableBoolParam(L,-1,&sync,"sync");

		MrcpParams p;
		FillTable(L,-1,p);

		ApiErrorCode res  = _mrcpSession->Recognize(p, body,Seconds(secs),sync);
		lua_pushnumber (L, res);

		

		return 1;


	}

	int 
	MrcpBridge::modify(lua_State *L)
	{
		return 0;

	}

	int 
	MrcpBridge::stopspeak(lua_State *L)
	{
		return 0;

	}

	int 
	MrcpBridge::speak(lua_State *L)
	{FUNCTRACKER;

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
	GetTableStringParam(L,-1,rawbody,"raw_body");

	MrcpParams p;
	FillTable(L,-1,p);

	stringstream mrcp_body;
	if (!sentence.empty())
	{
		mrcp_body 
			<< "<?xml version=\"1.0\"?>									" << endl 
			<< "	<speak>												" << endl
			<< "		<paragraph>										" << endl
			<< "			<sentence>" << sentence << "</sentence>		" << endl
			<< "		</paragraph>									" << endl
			<< "	</speak>											" << endl;	
	} else
	{
		mrcp_body << rawbody;
	}
	


	ApiErrorCode res =_mrcpSession->Speak(p,mrcp_body.str(),sync);

	lua_pushnumber (L, res);
	return 1;

	}

	int 
	MrcpBridge::teardown(lua_State *L)
	{
		return 0;

	}



}


/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "StdAfx.h"
#include "BridgeMacros.h"
#include "CallBridge.h"

namespace ivrworx
{
	const char CallBridge::className[] = "CallBridge";
	Luna<CallBridge>::RegType CallBridge::methods[] = {
		method(CallBridge, answer),
		method(CallBridge, speak),
		method(CallBridge, speakmrcp),
		method(CallBridge, cleandtmfbuffer),
		method(CallBridge, waitfordtmf),
		method(CallBridge, stopspeak),
		method(CallBridge, play),
		method(CallBridge, stopplay),
		method(CallBridge, waitforhangup),
		method(CallBridge, ani),
		method(CallBridge, dnis),
		method(CallBridge, mediaformat),
		method(CallBridge, makecall),
		method(CallBridge, blindxfer),
		method(CallBridge, hangup),
		method(CallBridge, rtspsetup),
		method(CallBridge, rtspplay),
		method(CallBridge, rtsppause),
		method(CallBridge, rtspteardown),
		method(CallBridge, sendinfo),
		{0,0}
	};

	CallBridge::CallBridge(lua_State *L)
	{
		
	}

	CallBridge::CallBridge(CallWithRtpManagementPtr call):
	_call(call)
	{
		
	}

	CallBridge::~CallBridge(void)
	{
		_call.reset();
	}

	int 
	CallBridge::play(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}
		

		LUA_BOOL_PARAM(L,loop,-1);
		LUA_BOOL_PARAM(L,sync,-2);
		LUA_STRING_PARAM(L,file_to_play,-3);

		ApiErrorCode res = _call->PlayFile(file_to_play,sync,loop);
		lua_pushnumber (L, res);

		return 1;

	}

	int
	CallBridge::stopplay(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		ApiErrorCode res = _call->StopPlay();
		lua_pushnumber (L, res);

		return 1;
	}

	int
	CallBridge::hangup(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		ApiErrorCode res = _call->HangupCall();
		lua_pushnumber (L, res);

		return 1;
	}

	int
	CallBridge::makecall(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_STRING_PARAM(L,dest,-1);

		ApiErrorCode res = _call->MakeCall(dest);
		lua_pushnumber (L, res);

		return 1;
	}

	int
	CallBridge::blindxfer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_STRING_PARAM(L,dest,-1);

		ApiErrorCode res = _call->BlindXfer(dest);
		lua_pushnumber (L, res);

		return 1;
	}


	int
	CallBridge::stopspeak(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LogDebug("CallBridge::cleandtmfbuffer iwh:" << _call->StackCallHandle());

		_call->StopSpeak();

		lua_pushnumber (L, API_SUCCESS);
		return 1;

	}

	int
	CallBridge::cleandtmfbuffer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}
		
		LogDebug("CallBridge::cleandtmfbuffer iwh:" << _call->StackCallHandle());

		_call->CleanDtmfBuffer();

		lua_pushnumber (L, API_SUCCESS);
		return 1;

	}

	int
	CallBridge::waitfordtmf(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		
		LUA_INT_PARAM(L,timeout,-1);
		
		LogDebug("CallBridge::waitfordtmf iwh:" << _call->StackCallHandle() << ", timeout:" << timeout);

		string signal;
#pragma TODO ("CSP++ Does not handle correctly the large timeouts")
#pragma warning (suppress:4244)
		ApiErrorCode res = 	_call->WaitForDtmf(signal, MilliSeconds(timeout));

		lua_pushnumber (L, res);
		if (IW_SUCCESS(res))
		{
			lua_pushstring(L, signal.c_str());
		} 
		else
		{
			lua_pushnil(L);
		}

		return 2;

	}

	int
	CallBridge::speakmrcp(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_BOOL_PARAM(L,sync,-1);
		LUA_STRING_PARAM(L,mrcp_string,-2);

		LogDebug("CallBridge::speakmrcp  iwh:" << _call->StackCallHandle() << ", speak:" << mrcp_string << ", sync:" << sync);
		ApiErrorCode res =_call->Speak(mrcp_string,sync);

		lua_pushnumber (L, res);
		return 1;

	}

	int
	CallBridge::speak(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_BOOL_PARAM(L,sync,-1);
		LUA_STRING_PARAM(L,mrcp_string,-2);

		LogDebug("CallBridge::speak  iwh:" << _call->StackCallHandle() << ", speak:" << mrcp_string << ", sync:" << sync);

		stringstream mrcp_body;
		mrcp_body 
			<< "<?xml version=\"1.0\"?>									" << endl 
			<< "	<speak>												" << endl
			<< "		<paragraph>										" << endl
			<< "			<sentence>" << mrcp_string << "</sentence>	" << endl
			<< "		</paragraph>									" << endl
			<< "	</speak>											" << endl;	


		ApiErrorCode res =_call->Speak(mrcp_body.str(),sync);

		lua_pushnumber (L, res);
		return 1;

	}

	int
	CallBridge::ani(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L,_call->Ani().c_str());

		return 1;
		
	}

	int
	CallBridge::mediaformat(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L,_call->AcceptedSpeechCodec().sdp_name_tos().c_str());

		return 1;

	}

	int
	CallBridge::dnis(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L,_call->Dnis().c_str());

		return 1;
	}

	int
	CallBridge::waitforhangup(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		_call->WaitTillHangup();

		lua_pushnumber (L, API_SUCCESS);
		return 1;
	}

	int
	CallBridge::answer(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LogDebug("CallBridge::answer  iwh:" << _call->StackCallHandle());

		ApiErrorCode res = _call->AcceptInitialOffer();
		lua_pushnumber (L, res);

		return 1;
	}

	int
	CallBridge::rtspsetup(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_STRING_PARAM(L,rtsp_url,-1);
		LogDebug("CallBridge::rtspsetup iwh:" << _call->StackCallHandle() << "rtsp url:" << rtsp_url);

		ApiErrorCode res = _call->RtspSetup(rtsp_url);
		lua_pushnumber (L, res);

		return 1;

	}

	int
	CallBridge::rtspplay(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_DOUBLE_PARAM(L,start_time,-3);
		LUA_DOUBLE_PARAM(L,duration,-2);
		LUA_DOUBLE_PARAM(L,scale,-1);

		LogDebug("CallBridge::rtspplay iwh:" << _call->StackCallHandle() << "start:" << start_time << ", end:" << duration << ", scale:" << scale );

		ApiErrorCode res = _call->RtspPlay(start_time,duration, (float)scale);
		lua_pushnumber (L, res);

		return 1;

	}

	int
	CallBridge::rtsppause(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LogDebug("CallBridge::rtsppause iwh:" << _call->StackCallHandle() );

		ApiErrorCode res = _call->RtspPause();
		lua_pushnumber (L, res);

		return 1;

	}

	int
	CallBridge::rtspteardown(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LogDebug("CallBridge::rtspteardown iwh:" << _call->StackCallHandle() );

		ApiErrorCode res = _call->RtspTearDown();
		lua_pushnumber (L, res);

		return 1;

	}

	int
	CallBridge::sendinfo(lua_State *L)
	{
		FUNCTRACKER;

		if (!_call)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		LUA_STRING_PARAM(L,body,-2);
		LUA_STRING_PARAM(L,type,-1);


		ApiErrorCode res = _call->SendInfo(body,type);
		lua_pushnumber (L, res);

		return 1;

	}


}



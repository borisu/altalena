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
#include "StreamerBridge.h"
#include "StreamingSession.h"
#include "StreamerBridge.h"
#include "BridgeMacros.h"

namespace ivrworx
{
	const char streamer::className[] = "streamer";
	Luna<streamer>::RegType streamer::methods[] = {
		method(streamer, allocate),
		method(streamer, play),
		method(streamer, modify),
		method(streamer, localoffer),
		method(streamer, remoteoffer),
		method(streamer, stop),
		{0,0}
	};

	streamer::streamer(lua_State *L)
	{
	}

	
	streamer::streamer(StreamingSessionPtr ptr):
	_streamingSessionPtr(ptr)
	{

	}

	streamer::~streamer(void)
	{
	}

	int streamer::stop(lua_State *L)
	{
		if (!_streamingSessionPtr)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		ApiErrorCode res = _streamingSessionPtr->StopPlay();

		lua_pushnumber (L, res);

		return 1;

	}

	int 
	streamer::allocate(lua_State *L)
	{
		if (!_streamingSessionPtr)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		AbstractOffer offer;
		BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
		offer.type = "sdp";
	
		RcvDeviceType rcv_type = RCV_DEVICE_NONE;
		GetTableEnumParam(L,-1,&rcv_type,"rcv");

		SndDeviceType snd_type = SND_DEVICE_TYPE_FILE;
		GetTableEnumParam(L,-1,&snd_type,"snd");

		ApiErrorCode res = API_SERVER_FAILURE;

		res  = _streamingSessionPtr->Allocate(
			offer, 
			rcv_type,
			snd_type);

		lua_pushnumber (L, res);

		return 1;

	}

	

	int 
	streamer::modify(lua_State *L)
	{
		if (!_streamingSessionPtr)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		AbstractOffer offer;
		BOOL paramres = GetTableStringParam(L,-1,offer.body,"sdp");
		offer.type = "sdp";

		if (!paramres)
		{
			lua_pushnumber (L, API_WRONG_PARAMETER);
			return 1;

		}
		
		ApiErrorCode res  = API_SERVER_FAILURE;
			_streamingSessionPtr->ModifyConnection(offer);

		lua_pushnumber (L, res);
		return 1;

	}

	int  
	streamer::play(lua_State *L)
	{
		if (!_streamingSessionPtr)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		string file ;
		BOOL paramres = GetTableStringParam(L,-1,file,"file");
		

		bool sync = true;
		GetTableBoolParam(L,-1,&sync,"sync");

		bool loop = false;
		GetTableBoolParam(L,-1,&loop,"loop");
		

		ApiErrorCode res =
			_streamingSessionPtr->PlayFile(file,sync,loop);

		lua_pushnumber (L, res);
		return 1;

	}

	int
	streamer::remoteoffer(lua_State *L)
	{
		FUNCTRACKER;
		if (!_streamingSessionPtr)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _streamingSessionPtr->RemoteOffer().body.c_str());
		return 1;
	}

	int
	streamer::localoffer(lua_State *L)
	{
		FUNCTRACKER;
		if (!_streamingSessionPtr)
		{
			lua_pushnumber (L, API_WRONG_STATE);
			return 1;
		}

		lua_pushstring(L, _streamingSessionPtr->LocalOffer().body.c_str());
		return 1;
	}
	


}


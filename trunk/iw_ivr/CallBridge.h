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

#pragma once
#include "Luna.h"
#include "CallWithRtpManagement.h"


namespace ivrworx
{
	class CallBridge
	{
	public:

		CallBridge(lua_State *L);

		CallBridge(CallWithRtpManagementPtr call);

		virtual ~CallBridge(void);

		int answer(lua_State *L);
		int speak(lua_State *L);
		int speakmrcp(lua_State *L);
		int cleandtmfbuffer(lua_State *L);
		int waitfordtmf(lua_State *L);
		int stopspeak(lua_State *L);
		int play(lua_State *L);
		int stopplay(lua_State *L);
		int waitforhangup(lua_State *L);
		int ani(lua_State *L);
		int dnis(lua_State *L);

		/**
		
			Calls SIP destination. 

			\code
			call = ivrworx.createcall();
			call:makecall("sip:6095@192.168.150.3:5060");
			\endcode

			@return ApiErrorCode

		**/
		int makecall(lua_State *L);
		int blindxfer(lua_State *L);
		int hangup(lua_State *L);

		static const char className[];
		static Luna<CallBridge>::RegType methods[];

	private:

		CallWithRtpManagementPtr _call;
	};


}


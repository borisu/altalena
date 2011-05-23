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

namespace ivrworx
{
	class sipcall
	{
	public:

		sipcall(lua_State *L);

		sipcall(SipMediaCallPtr call);

		virtual ~sipcall(void);


		int accept(lua_State *L);

		int answer(lua_State *L);
		int cleandtmfbuffer(lua_State *L);
		int waitfordtmf(lua_State *L);
		int waitforhangup(lua_State *L);
		int ani(lua_State *L);
		int dnis(lua_State *L);
		

		int sendinfo(lua_State *L);
		int waitforinfo(lua_State *L);
		int cleaninfobuffer(lua_State *L);

		int makecall(lua_State *L);
		int blindxfer(lua_State *L);
		int hangup(lua_State *L);

	
		int remoteoffer(lua_State *L);
		int localoffer(lua_State *L);

		// register is a reserved word in C/C++ 
		int startregister(lua_State *L);
		int unregister(lua_State *L);

		int reoffer(lua_State *L);

		int subscribe(lua_State *L);
		int unsubscribe(lua_State *L);
		int waitfornotify(lua_State *L);
		int cleannotifybuffer(lua_State *L);

		static const char className[];
		static Luna<sipcall>::RegType methods[];

	private:

		SipMediaCallPtr _call;
	};


}

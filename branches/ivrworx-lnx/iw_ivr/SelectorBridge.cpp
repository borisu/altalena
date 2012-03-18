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
#include "SelectorBridge.h"
#include "LuaStaticApi.h"


namespace ivrworx
{
	const char selector::className[] = "selector";
	Luna<selector>::RegType selector::methods[] = {
		method(selector, select),
		{0,0}
	};

	selector::selector(lua_State *L):
	_hv(new HandlesVector())
	{

	}

	int
	selector::select(lua_State *L)
	{
		int top = lua_gettop(L);
		if (top !=1)
		{
			lua_pushnumber(L,API_WRONG_PARAMETER);
			return 1;
		}

		if (_hv->size() == 0)
		{
			if (lua_istable(L,-1) == FALSE)
			{
				lua_pushnumber(L,API_WRONG_PARAMETER);
				return 1;
			}


			// find named parameter "actors" and ensure it is table
			lua_pushstring(L, "actors");
			lua_gettable(L, -2);
			if (lua_istable(L,-1) == FALSE)
			{
				lua_settop(L,top);
				lua_pushnumber(L,API_WRONG_PARAMETER);
				return 1;
			}


			int param_counter = 0;

			lua_pushnil(L);
			while(lua_next(L,-2)) 
			{
				switch (lua_type(L,-1))
				{
				case LUA_TUSERDATA:
					{
						userdataType *x = (userdataType *)lua_touserdata(L,-1);
						luaobject *lo = x->lo;

						if (lo->get_active_object())
						{

							AOSlot slot;
							slot.h  = LpHandlePtr(new LpHandle());
							slot.h->HandleName("Active Object Listener");
							slot.ao = lo->get_active_object();
							_hv->push_back(slot.h);

							lo->get_active_object()->AddEventListener(slot.h);

							_slots.push_back(slot);

						}
						else
						{
							lua_settop(L,top);
							lua_pushnumber(L,API_WRONG_PARAMETER);
							return 1;
						}
						break;
					}
				default:
					{
						lua_settop(L,top);
						lua_pushnumber(L,API_WRONG_PARAMETER);
						return 1;
					}
				}
				lua_pop(L, 1);
			}

			// pop the actors table from the stack
			lua_pop(L,1);
		}
		

		if (_hv->size() == 0)
		{
			lua_settop(L,top);
			lua_pushnumber(L,API_WRONG_PARAMETER);
			return 1;
		}

		
		int timeout = 35;
		GetTableNumberParam(L,-1,&timeout,"timeout",35);

		ApiErrorCode err	= API_SUCCESS;
		int selected_index  = -1;

		IwMessagePtr event;
		err = SelectFromChannels(
			*_hv, 
			Seconds(timeout),
			selected_index,
			event);

		lua_pushnumber(L,err);

		if (IW_SUCCESS(err))
		{
			lua_pushnumber(L,selected_index+1);

		} 
		else
		{
			lua_pushnumber(L,-1);
		};

		// unset listeners

		return 2;


	}

	selector::~selector(void)
	{
		for (list<AOSlot>::iterator iter = _slots.begin(); iter != _slots.end(); ++iter)
		{
			AOSlot &slot = (*iter);
			slot.ao->RemoveEventListener(slot.h);
		};

		_hv->clear();
		_slots.clear();
	}
}
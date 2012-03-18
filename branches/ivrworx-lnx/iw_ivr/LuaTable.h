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
#include "IvrDllApi.h"

namespace ivrworx
{
	class IW_IVR_API LuaTable
	{
	public:

		LuaTable(IN lua_State  *L);

		void Create(IN const string &table_name);

		void Attach(IN const string &name);

		void AddParam(IN const string &key,  IN const string &value);

		void AddParam(IN const string &key,  IN const int value);

		void AddFunction(IN const string &key,  IN const lua_CFunction func);

		int TableRef();
			  
		virtual ~LuaTable (void);

	protected:

		string _tableName;

		int _oldRef;

		lua_State  *_vm;

		int _tableRef;

	};

}
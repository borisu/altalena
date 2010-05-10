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

#include "LuaVirtualMachine.h"
#include "LuaScript.h"
#include "CallWithRtpManagement.h"
#include "LuaTable.h"


namespace ivrworx
{

	class IwScript :
		public CLuaScript
	{
	public:
		IwScript(CLuaVirtualMachine &vm);

		// When the script calls a class method, this is called
		virtual int ScriptCalling (CLuaVirtualMachine& vm, int iFunctionNumber);
		
		// When the script function has returns
		virtual void HandleReturns (CLuaVirtualMachine& vm, const char *strFunc);

	};


	enum IvrEvents
	{
		MSG_IVR_START_SCRIPT_REQ = MSG_USER_DEFINED,
	};

	enum ParamType
	{
		PT_UNKNOWN,
		PT_INTEGER,
		PT_STRING,
		PT_BOOL,
		PT_CALL,
		PT_LIGHTUSERDATA
	};

	struct ProcParam 
	{
		ParamType param_type; 

		ProcParam():
		param_type(PT_UNKNOWN),
			ud_value(NULL)
		{

		}

		ProcParam(const ProcParam &p)
		{
			int_value    = p.int_value;
			string_value = p.string_value;
			call_value	 = p.call_value;
			bool_value	 = p.bool_value;
			ud_value	 = p.ud_value;

		}

		/* couldn't use union because of copy c'tor */
		int int_value;

		BOOL bool_value;

		void *ud_value;

		string string_value;

		CallWithRtpManagementPtr call_value;

	};


	typedef map<int,ProcParam>  
	ProcParamMap;

	typedef shared_ptr<ProcParamMap>
	ProcParamMapPtr;


	class MsgIvrStartScriptReq : 
		public MsgRequest
	{
	public:
		MsgIvrStartScriptReq():
		  MsgRequest(MSG_IVR_START_SCRIPT_REQ, 
			  NAME(MSG_IVR_START_SCRIPT_REQ)){};

		  string script_name;

		  ProcParamMapPtr params_map;

	};
	

	/**
	Main Ivr process implementation
	**/
	class ProcScriptRunner
		: public LightweightProcess
	{
	public:

		ProcScriptRunner(
			IN Configuration &conf,
			IN const string &script_name,
			IN const char *precompiled_buffer,
			IN size_t buffer_size,
			IN shared_ptr<MsgCallOfferedReq> msg, 
			IN LpHandlePair ivr_pair, 
			IN LpHandlePair pair);

		ProcScriptRunner(
			IN Configuration &conf,
			IN shared_ptr<MsgIvrStartScriptReq> req,
			IN LpHandlePair ivr_pair, 
			IN LpHandlePair pair);

		~ProcScriptRunner();

		virtual void real_run();

		virtual BOOL RunScript(IwScript &script);

		virtual BOOL HandleOOBMessage(IN IwMessagePtr msg);

		static int LuaWait(lua_State *L);

		static int LuaRunLongOperation(lua_State *L);

		static int LuaCreateCall(lua_State *L);

		static int LuaCreateMscmlCall(lua_State *L);

		static int LuaSpawn(lua_State *L);

		static int LuaFork(lua_State *L);

	private:

		shared_ptr<MsgCallOfferedReq> _initialMsg;

		shared_ptr<MsgIvrStartScriptReq> _startScriptReq;

		LpHandlePair _spawnPair;

		Configuration &_conf;

		int _stackHandle;

		const string _scriptName;

		const char *_precompiledBuffer;

		size_t _bufferSize;

		ScopedForking *_forking;

		ProcParamMap _procMap;

	};


	
}

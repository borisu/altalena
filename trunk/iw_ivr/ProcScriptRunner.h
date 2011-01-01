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
#include "LuaTable.h"
#include "LuaStaticApi.h"

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

	

	/**
	Main Ivr process implementation
	**/
	class ProcScriptRunner
		: public LightweightProcess
	{
	public:

		ProcScriptRunner(
			IN ConfigurationPtr conf,
			IN const string &script_name,
			IN const char *precompiled_buffer,
			IN size_t buffer_size,
			IN shared_ptr<MsgCallOfferedReq> msg, 
			IN LpHandlePair ivr_pair, 
			IN LpHandlePair pair);

		~ProcScriptRunner();

		virtual void real_run();

		virtual BOOL RunScript(IwScript &script);

		virtual BOOL HandleOOBMessage(IN IwMessagePtr msg);

		virtual AppData* GetAppData() { return &_ctx; };

	
	private:

		Context _ctx;

		shared_ptr<MsgCallOfferedReq> _initialMsg;

		LpHandlePair _spawnPair;

		ConfigurationPtr _conf;

		int _stackHandle;

		const string _scriptName;

		const char *_precompiledBuffer;

		size_t _bufferSize;

		ScopedForking *_forking;

		void Init();

	};


	
}

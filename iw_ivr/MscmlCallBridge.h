#pragma once
#include "MscmlCall.h"
#include "Luna.h"

namespace ivrworx
{
	

	class MscmlCallBridge 
	{
	public:

		MscmlCallBridge(lua_State *L);
		MscmlCallBridge(MscmlCallPtr call);

		~MscmlCallBridge();

		// Lua interface
		int setup(lua_State *L);
		int hangup(lua_State *L);
		int sendrawrequest(lua_State *L);


		static const char className[];
		static Luna<MscmlCallBridge>::RegType methods[];

	private:

		MscmlCallPtr _call;

	};


}


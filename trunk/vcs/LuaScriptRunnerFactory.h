#pragma once
#include "LightweightProcess.h"

class LuaScriptRunnerFactory
{
public:
	LuaScriptRunnerFactory(void);
	virtual ~LuaScriptRunnerFactory(void);

	static LightweightProcess *CreateScriptRunner(IN LpHandlePair pair, IN LpHandlePair stack_pair);
};

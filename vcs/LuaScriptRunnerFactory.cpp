#include "StdAfx.h"
#include "LuaScriptRunnerFactory.h"
#include "ProcLuaScriptRunner.h"

LuaScriptRunnerFactory::LuaScriptRunnerFactory(void)
{
}

LuaScriptRunnerFactory::~LuaScriptRunnerFactory(void)
{
}

LightweightProcess*
LuaScriptRunnerFactory::CreateScriptRunner(IN LpHandlePair pair, IN LpHandlePair stack_pair)
{
	return new ProcLuaScriptRunner(pair,stack_pair);
}

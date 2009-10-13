// ---------------------------------------------------------------------------
// FILE NAME            : LuaVirtualMachine.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Lua virtual machine implementation
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES
#ifndef __LUA_VIRTUAL_MACHINE_H__
#define __LUA_VIRTUAL_MACHINE_H__

//#include "lualib/luainc.h"
#include "luadebugger.h"


using namespace ivrworx;

class CLuaDebugger;

class CLuaVirtualMachine
{
public:
   CLuaVirtualMachine (void);
   virtual ~CLuaVirtualMachine (void);

   bool InitialiseVM (void);
   bool DestroyVM (void);

   // Load and run script elements
   bool RunFile (const char *strFilename);
   bool RunBuffer (const unsigned char *pbBuffer, size_t szLen, const char *strName = NULL);

   // C-Api into script
   bool CallFunction (int nArgs, int nReturns = 0);

   // Get the state of the lua stack (use the cast operator)
   //lua_State *GetState (void) { return m_pState; }
   operator lua_State *(void) { return m_pState; }

   static void Panic (lua_State *lua);

   // Check if the VM is OK and can be used still
   virtual bool Ok (void) { return m_fIsOk; }

   // For debugging
   void AttachDebugger (CLuaDebugger *dbg) { m_pDbg = dbg; }

protected:
   lua_State *m_pState;
   bool m_fIsOk;
   CLuaDebugger *m_pDbg;
};


#endif // __LUA_VIRTUAL_MACHINE_H__
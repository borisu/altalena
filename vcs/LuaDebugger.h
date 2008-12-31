// ---------------------------------------------------------------------------
// FILE NAME            : LuaDebugger.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Debugging interface
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES
#ifndef __LUA_DEBUGGER_H__
#define __LUA_DEBUGGER_H__

//#include "lualib/luainc.h"
#include "luavirtualmachine.h"

enum
{
   DBG_MASK_CALL = LUA_MASKCALL,
   DBG_MASK_RET = LUA_MASKRET,
   DBG_MASK_LINE = LUA_MASKLINE,
   DBG_MASK_COUNT = LUA_MASKCOUNT
};

class CLuaVirtualMachine;

class CLuaDebugger
{
public:
   CLuaDebugger (CLuaVirtualMachine& vm);
   virtual ~CLuaDebugger (void);

   void SetHooksFlag (int iMask);
   void SetCount (int iCount) { m_iCountMask = iCount; }

   void ErrorRun (int iErrorCode);

protected:
   int m_iCountMask;
   CLuaVirtualMachine& m_vm;
};


#endif // __LUA_DEBUGGER_H__
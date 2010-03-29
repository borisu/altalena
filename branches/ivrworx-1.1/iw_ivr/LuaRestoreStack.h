// ---------------------------------------------------------------------------
// FILE NAME            : LuaRestoreStack.h
// ---------------------------------------------------------------------------
// DESCRIPTION :
//
// Restores the Lua stack to the way we found it :)
// 
// ---------------------------------------------------------------------------
// VERSION              : 1.00
// DATE                 : 1-Sep-2005
// AUTHOR               : Richard Shephard
// ---------------------------------------------------------------------------
// LIBRARY INCLUDE FILES
#ifndef __LUA_RESTORE_STACK_H__
#define __LUA_RESTORE_STACK_H__

#include "LuaVirtualMachine.h"

class CLuaRestoreStack
{
public:
   CLuaRestoreStack (CLuaVirtualMachine& vm) : m_pState (NULL)
   {
      m_pState = (lua_State *) vm;
      if (vm.Ok ())
      {
         m_iTop = lua_gettop (m_pState);
      }
   }

   CLuaRestoreStack (lua_State *state) : m_pState (NULL)
   {
	   m_pState = state;
	   m_iTop = lua_gettop (m_pState);
   }

   virtual ~CLuaRestoreStack (void)
   {
      lua_settop (m_pState, m_iTop);
   }

protected:
   lua_State *m_pState;
   int m_iTop;
};


#endif // __LUA_RESTORE_STACK_H__
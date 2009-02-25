// ---------------------------------------------------------------------------
// FILE NAME            : LuaVirtualMachine.cpp
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
//#include "luaLib/luainc.h"
#include "StdAfx.h"
#include "luavirtualmachine.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
// ---------------------------------------------------------------------------


//============================================================================
// int printMessage
//---------------------------------------------------------------------------
// Prints a message to the console
//
// Parameter   Dir      Description
// ---------   ---      -----------
// lua         IN       State variable
//
// Return
// ------
// Number of return varaibles on the stack
//
//============================================================================
static int printMessage (lua_State *lua)
{
   assert (lua_isstring (lua,1));

   const char *msg = lua_tostring (lua, 1);

   // get caller
   lua_Debug ar;
   memset (&ar, 0, sizeof(ar));
   lua_getstack (lua, 1, &ar);
   lua_getinfo (lua, "Snl", &ar);

   // debug output
   const char *str = ar.source;
   printf ("script: %s -- at %s(%d)\n", msg, str, ar.currentline);
   return 0;
}

//============================================================================
// CLuaVirtualMachine::CLuaVirtualMachine
//---------------------------------------------------------------------------
// Constructor. Setups the default VM state
//
// Parameter   Dir      Description
// ---------   ---      -----------
// 
//
// Return
// ------
// None.
//
//============================================================================
CLuaVirtualMachine::CLuaVirtualMachine (void) : m_pState (NULL), m_pDbg (NULL)
{
   m_fIsOk = false;
}

//============================================================================
// CLuaVirtualMachine::CLuaVirtualMachine
//---------------------------------------------------------------------------
// Destructor. Closes the VM
//
// Parameter   Dir      Description
// ---------   ---      -----------
// 
//
// Return
// ------
// None.
//
//============================================================================
CLuaVirtualMachine::~CLuaVirtualMachine (void)
{
   if (m_pState != NULL)
   {
      lua_close (m_pState);
   }
}

//============================================================================
// CLuaVirtualMachine::Panic
//---------------------------------------------------------------------------
// When things in Lua go wrong (ever called in protected mode??)
//
// Parameter   Dir      Description
// ---------   ---      -----------
// lua         IN       State variable
//
// Return
// ------
// None.
//
//============================================================================
void CLuaVirtualMachine::Panic (lua_State *lua)
{
}

//============================================================================
// bool CLuaVirtualMachine::InitialiseVM
//---------------------------------------------------------------------------
// Initialises the VM, open lua, makes sure things are OK
//
// Parameter   Dir      Description
// ---------   ---      -----------
// None.
//
// Return
// ------
// Success.
//
//============================================================================
bool CLuaVirtualMachine::InitialiseVM (void)
{
   // Open Lua!
   if (Ok ()) DestroyVM ();

   m_pState = lua_open ();

   if (m_pState) 
   {
      m_fIsOk = true;

      // Load util libs into lua
//       luaopen_base (m_pState);
//       luaopen_table (m_pState);
//       luaopen_string (m_pState);
//       luaopen_math (m_pState);
//       luaopen_debug (m_pState);
//       luaopen_io (m_pState);
      luaL_openlibs (m_pState);
	  

      // setup global printing (trace)
      lua_pushcclosure (m_pState, printMessage, 0);
      lua_setglobal (m_pState, "trace");

      lua_atpanic (m_pState, (lua_CFunction) CLuaVirtualMachine::Panic);

      return true;
   }

   return false;
}

//============================================================================
// bool CLuaVirtualMachine::DestroyVM
//---------------------------------------------------------------------------
// Clears the current Lua state
//
// Parameter   Dir      Description
// ---------   ---      -----------
// None.
//
// Return
// ------
// Success.
//
//============================================================================
bool CLuaVirtualMachine::DestroyVM (void)
{
   if (m_pState)
   {
      lua_close (m_pState);
      m_pState = NULL;
      m_fIsOk = false;
   }
   return true;
}


//============================================================================
// bool CLuaVirtualMachine::RunFile 
//---------------------------------------------------------------------------
// Compiles and runs a lua script file
//
// Parameter   Dir      Description
// ---------   ---      -----------
// strFilename IN       Filename to compile and run
//
// Return
// ------
// Success.
//
//============================================================================
bool CLuaVirtualMachine::RunFile (const char *strFilename)
{
   bool fSuccess = false;
   int iErr = 0;

   if ((iErr = luaL_loadfile (m_pState, strFilename)) == 0)
   {
      // Call main...
       if ((iErr = lua_pcall (m_pState, 0, LUA_MULTRET, 0)) == 0)
       {
          fSuccess = true;
       }
   }

   if (fSuccess == false)
   {
      if (m_pDbg != NULL) m_pDbg->ErrorRun (iErr);
   }

   return fSuccess;
}

//============================================================================
// bool CLuaVirtualMachine::RunBuffer 
//---------------------------------------------------------------------------
// Compiles and runs a pre-compiled data buffer
//
// Parameter   Dir      Description
// ---------   ---      -----------
// pbBuffer    IN       Buffer to run
// szLen       IN       Length of buffer
// strName     IN       Name of Buffer
//
// Return
// ------
// Success.
//
//============================================================================
bool CLuaVirtualMachine::RunBuffer (const unsigned char *pbBuffer, size_t szLen, const char *strName /* = NULL */)
{
   bool fSuccess = false;
   int iErr = 0;

   if (strName == NULL)
   {
      strName = "Temp";
   }

   if ((iErr = luaL_loadbuffer (m_pState, (const char *) pbBuffer, szLen, strName)) == 0)
   {
      // Call main...
       if ((iErr = lua_pcall (m_pState, 0, LUA_MULTRET, 0)) == 0)
       {
          fSuccess = true;
       }
   }

   if (fSuccess == false)
   {
      if (m_pDbg != NULL) m_pDbg->ErrorRun (iErr);
   }

   return fSuccess;   

}

//============================================================================
// CLuaVirtualMachine::CallFunction 
//---------------------------------------------------------------------------
// Calls a function that is already on the stack
//
// Parameter   Dir      Description
// ---------   ---      -----------
// nArgs       IN       Args that are aleady on the stack
// nReturns    IN       Number of expected returns (will be on the stack)
//
// Return
// ------
// Success.
//
//============================================================================
bool CLuaVirtualMachine::CallFunction (int nArgs, int nReturns /* = 0 */)
{
   bool fSuccess = false;
   
   if (lua_isfunction (m_pState, -nArgs-1))
   {
      int iErr = 0;
      iErr = lua_pcall (m_pState, nArgs, nReturns, 0);

      if (iErr == 0)
      {
         fSuccess = true;
      }
      else
      {
         if (m_pDbg != NULL) m_pDbg->ErrorRun (iErr);
      }
   }

   return fSuccess;
}


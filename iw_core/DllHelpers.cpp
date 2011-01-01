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

#include "stdafx.h"
#include "DllHelpers.h"

static DWORD	dwTlsIndex = 0; // address of shared memory
static LONG		lMaxSlots = 256;
static LONG		lTlsSize =  sizeof(LPVOID)*lMaxSlots;

// DllMain() is the entry-point function for this DLL. 
// http://msdn.microsoft.com/en-us/library/ms686997%28VS.85%29.aspx

BOOL WINAPI DllMain(HINSTANCE hinstDLL, // DLL module handle
					DWORD fdwReason,                    // reason called
					LPVOID lpvReserved)                 // reserved
{ 
	LPVOID lpvThreadData; 
	BOOL fIgnore; 

	switch (fdwReason) 
	{ 
		// The DLL is loading due to process 
		// initialization or a call to LoadLibrary. 

	case DLL_PROCESS_ATTACH: 

		// Allocate a TLS index.

		if ((dwTlsIndex = TlsAlloc()) == TLS_OUT_OF_INDEXES) 
			return FALSE; 

		// No break: Initialize the index for first thread.

		// The attached process creates a new thread. 

	case DLL_THREAD_ATTACH: 

		// Initialize the TLS index for this thread.

		lpvThreadData = (LPVOID) LocalAlloc(LPTR, lTlsSize); 
		if (lpvThreadData != NULL) 
			fIgnore = TlsSetValue(dwTlsIndex, lpvThreadData); 

		break; 

		// The thread of the attached process terminates.

	case DLL_THREAD_DETACH: 

		// Release the allocated memory for this thread.

		lpvThreadData = TlsGetValue(dwTlsIndex); 
		if (lpvThreadData != NULL) 
			LocalFree((HLOCAL) lpvThreadData); 

		break; 

		// DLL unload due to process termination or FreeLibrary. 

	case DLL_PROCESS_DETACH: 

		// Release the allocated memory for this thread.

		lpvThreadData = TlsGetValue(dwTlsIndex); 
		if (lpvThreadData != NULL) 
			LocalFree((HLOCAL) lpvThreadData); 

		// Release the TLS index.

		TlsFree(dwTlsIndex); 
		break; 

	default: 
		break; 
	} 

	return TRUE; 
	UNREFERENCED_PARAMETER(hinstDLL); 
	UNREFERENCED_PARAMETER(lpvReserved); 
}

IW_CORE_API
BOOL StoreCoreData(DWORD slotIndex, LPVOID slotValue)
{
	LPVOID lpvThreadData; 
	LPVOID * pData;  // The stored memory pointer 

	if (slotIndex < 0 || slotIndex > (lMaxSlots - 1))
		return false;

	lpvThreadData = TlsGetValue(dwTlsIndex); 
	if (lpvThreadData == NULL)
	{
		lpvThreadData = (LPVOID) LocalAlloc(LPTR, lTlsSize); 
		if (lpvThreadData == NULL) 
			return FALSE;
		if (!TlsSetValue(dwTlsIndex, lpvThreadData))
			return FALSE;
	}

	pData = &(((LPVOID *) lpvThreadData)[slotIndex]); // Cast to my data type.
	(*pData) = slotValue;

	return TRUE;
}

IW_CORE_API
BOOL GetCoreData(DWORD slotIndex,LPVOID *pdw)
{
	LPVOID lpvThreadData; 
	LPVOID pData;  // The stored memory pointer 

	if (slotIndex < 0 || slotIndex > (lMaxSlots - 1))
		return false;

	lpvThreadData = TlsGetValue(dwTlsIndex); 
	if (lpvThreadData == NULL)
		return FALSE;

	pData = ((LPVOID *)lpvThreadData)[slotIndex];
	(*pdw) = pData;
	return TRUE;
}

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

namespace ivrworx
{

#define IX_UNDEFINED -1

#define CCU_MAX_LONG_LENGTH 255
#define CCU_MAX_INT_LENGTH	255

	enum IxApiErrorCode
	{
		CCU_API_SUCCESS = 0,
		CCU_API_FAILURE,
		CCU_API_SERVER_FAILURE,
		CCU_API_TIMEOUT,
		CCU_API_OUT_OF_BAND,
		CCU_API_UNKNOWN_PROC_DESTINATION,
		CCU_API_OPERATION_IN_PROGRESS 
	};

#define CCU_SUCCESS(x)	((x) == CCU_API_SUCCESS)
#define CCU_FAILURE(x)	((x) != CCU_API_SUCCESS)


	typedef int CcuHandleId;

	typedef int IxProcId;

	typedef LARGE_INTEGER IxTimeStamp;


}



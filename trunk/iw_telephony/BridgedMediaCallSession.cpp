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
#include "StdAfx.h"
#include "BridgedMediaCallSession.h"


namespace ivrworx
{
	BridgedMediaCallSession::BridgedMediaCallSession(void)
	{
	}

	BridgedMediaCallSession::~BridgedMediaCallSession(void)
	{
	}

	ApiErrorCode
	BridgedMediaCallSession::MakeBridgedCall(
		MediaCallSessionPtr first_call,
		const string &first_destination,
		MediaCallSessionPtr second_call,
		const string &second_destination)
	{
		FUNCTRACKER;
		LogDebug("BridgedMediaCallSession::MakeBridgedCall - from:" << first_destination << " to:" << second_destination);

		if (!first_call					||
			!second_call				||
			first_destination.empty()   || 
			second_destination.empty() )
		{
			return API_WRONG_PARAMETER;
		};

		ApiErrorCode res = 
			first_call->MakeCall(first_destination,Seconds(35));
		if (res != API_PENDING_OPERATION)
		{
			return res;
		}

		res = second_call->MakeCall(second_destination,first_call->RemoteOffer(),Seconds(35));
		if (res != API_SUCCESS)
		{
			return res;
		}

		res = 
			first_call->Answer(second_call->RemoteOffer());


		return res;
		

	}


}


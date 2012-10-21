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
#include "AutoNative.h"

using namespace ivrworx;



namespace ivrworx
{
namespace interop
{
	typedef Dictionary<String^,String^> MapOfAnyInterop; 
	typedef System::Collections::Generic::LinkedList<String^> ListOfStrings;

	string MarshalToString ( const String ^ s); 

	wstring MarshalToWString ( const String ^ s);


	public enum ApiErrorCode
	{
		API_SUCCESS = 0,
		API_FAILURE,
		API_SOCKET_INIT_FAILURE,
		API_TIMER_INIT_FAILURE,
		API_SERVER_FAILURE,
		API_TIMEOUT,
		API_WRONG_PARAMETER,
		API_WRONG_STATE,
		API_HANGUP,
		API_UNKNOWN_DESTINATION,
		API_FEATURE_DISABLED,
		API_UNKNOWN_RESPONSE,
		API_PENDING_OPERATION
	};

	public ref class IvrWORX 
	{
	public:

		IvrWORX();

		void Init(String ^confName);

		~IvrWORX();

		property ScopedForking* Forking
		{
			ScopedForking* get() 
			{ 
				return _forking;
			}
		}


	private:

		long _threadId;

		ConfigurationPtr  *_conf;

		FactoryPtrList * _factoriesList;

		HandlePairList * _procHandles ;

		HandlesVector  * _shutdownHandles ;

		RunningContext *_ctx;

		ScopedForking  *_forking;

		bool _isDisposed;


	};

}
}




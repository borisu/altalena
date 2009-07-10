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

#include "LpHandle.h"
#include "LightweightProcess.h"

using namespace csp;
using namespace std;

namespace ivrworx
{
	typedef map<int,LpHandlePtr> EventListenersMap;

	/**

	@defgroup activeobject "Active Object"

	Implementation of Active Object Pattern.

	Suppose you have a class that represents a synchronous interface to some 
	service. However in addition to usual request response transactions, the
	service may send some unsolicited events. For example you are holding an
	object which represents some Call interface.

		@code
		class Call
		{
		...
		MakeCall();
		HangupCall();
		TransferCall();
		...
		}
		@endcode

	Obviously the call maybe hanged up by some third party at any time. The 
	object should handle this event, but as long as the object "lives" in context
	calling fiber (or thread for that matter) it will never receive the CPU time
	to handle the event. Providing that, you have to give CPU time for processing
	these unsolicited events. ActiveObject spawns new lightweight process in
	order to listen to such an events and handle them in its context. All events
	will be passed to UponActiveObjectEvent callback. The basic implementation
	dispatches the event to all listeners. It is in responsibility of inheriting
	class to implement application specific logic in this callback. So in our
	example the Call object should inherit the ActiveObject Call register it as
	call listener process and implement UponActiveObjectEvent callback.

		@code
		class Call : public ActiveObject
		{
		...
		MakeCall();
		HangupCall();
		TransferCall();

		UponActiveObjectEvent(evt)
		{
		if (evt == hangup) ...
		}
		...
		}
		@endcode

	Almost all session classes are using this logic to provide the callback for 
	unsolicited events.

	**/

	class ActiveObject	
	{
	public:

		/**
			
		Constructs the active object class.
		
		*/
		ActiveObject();

		/**
			
		Starts the listening light process.
		
		@param_in	forking - forking object of the parent lightweight process
		@param_in	pair - communication handle with the spawned light process
		@param_in	name - the name of the spawned light process
		
		*/
		virtual void StartActiveObjectLwProc(IN ScopedForking &forking, IN LpHandlePair pair, IN const string &name);

		
		/**
			
		Callback for events received in active object light process.
		
		@param_in	ptr - message received on incoming handle of pair passed in Start.
		
		**/
		virtual void UponActiveObjectEvent(IN IwMessagePtr ptr);

		/**
			
		Sets additional event listener for the active object events.
		
		@param_in	ccu_msg_id - id of the message to listen to.
		@param_in	listener_handle - handle to send the incoming events.
		
		**/
		virtual void SetEventListener(IN int ccu_msg_id, IN LpHandlePtr listener_handle);

		/**
		
		Destructor. Stops the fiber by sending it shutdown message. It will block till 
		process is stopped.
		
		*/
		virtual ~ActiveObject(void);

	private:

		LpHandlePair _handlePair;

		EventListenersMap _listenersMap;

		BucketPtr _listenerBucket;

		volatile BOOL _shutdownFlag;

		bool _started;

		friend class ProcEventListener;
	};

	/**
	
	The implementation of @ref activeobject listener.
	
	*/
	class ProcEventListener:
		public LightweightProcess
	{
	public:

		ProcEventListener(IN ActiveObject &object,
						  IN LpHandlePair pair, 
						  IN const string &name);

		void real_run();

	private:

		ActiveObject &_activeObject;

	};


}

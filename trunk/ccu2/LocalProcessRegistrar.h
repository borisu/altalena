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

using namespace std;
using namespace boost;

namespace ivrworx
{

	enum LocalProcessRegistrarEvts
	{
		MSG_PROC_SHUTDOWN_EVT = MSG_USER_DEFINED
	};

	class CcuMsgShutdownEvt: 
		public CcuMsgResponse
	{

	public:
		CcuMsgShutdownEvt():
		  CcuMsgResponse(MSG_PROC_SHUTDOWN_EVT, NAME(MSG_PROC_SHUTDOWN_EVT)),
			  proc_id(IW_UNDEFINED){};

		  CcuMsgShutdownEvt(ProcId pproc_id):
		  CcuMsgResponse(MSG_PROC_SHUTDOWN_EVT, NAME(MSG_PROC_SHUTDOWN_EVT)),
			  proc_id(pproc_id){};

		  ProcId proc_id;
	};




	class RegistrationGuard
		:public UIDOwner
	{
	public:

		RegistrationGuard(IN LpHandlePtr ptr, 
			IN int process_alias = IW_UNDEFINED);

		~RegistrationGuard();

	private:

		int _handleUid;

		int _aliasId;

	};

	class LocalProcessRegistrar
	{
		static mutex _instanceMutex;

		static LocalProcessRegistrar *_instance;

	private:

		typedef 
		map<int,LpHandlePtr> LocalProcessesMap;
		LocalProcessesMap _locProcessesMap;

		typedef
		map<ProcId, HandlesList> ListenersMap;
		ListenersMap _listenersMap;

		typedef
		map<int,int> AliasesMap;
		AliasesMap _aliasesMap;

		mutex _mutex;

	public:

		static LocalProcessRegistrar&  Instance();

		LocalProcessRegistrar(void);

		virtual ~LocalProcessRegistrar(void);

		void RegisterChannel(
			IN int channel_id, 
			IN LpHandlePtr hande);

		void UnregisterChannel(
			IN int channel_id);

		void AddShutdownListener(
			IN int channel_id, 
			IN LpHandlePtr listener_handle);

		LpHandlePtr GetHandle(
			IN int channel_id);

		LpHandlePtr GetHandle(
			IN int channel_id, 
			IN const string &qpath);
	};

}


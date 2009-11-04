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
using namespace boost;

#include "Ims.h"

namespace ivrworx
{

	typedef map<int,char> 
	DtmfMap;

	class ImsSession :
		public ActiveObject
	{
	public:

		ImsSession (IN ScopedForking &forking);

		virtual ~ImsSession (void);

		virtual ApiErrorCode	Allocate();

		virtual ApiErrorCode	Allocate(
			IN const CnxInfo &remote_end,
			IN const MediaFormat &codec);

		virtual ApiErrorCode	ModifyConnection(
			IN const CnxInfo &remote_end, 
			IN const MediaFormat &codec);

		virtual ApiErrorCode	StopPlay();

		virtual ApiErrorCode PlayFile(
			IN const string &file_name,
			IN BOOL sync = FALSE,
			IN BOOL loop = FALSE);

		virtual void TearDown();

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual void InterruptWithHangup();

		virtual ImsHandle SessionHandle();

	private:

		ApiErrorCode	SyncStreamFile();

		ImsHandle _imsSessionHandle;

		ScopedForking &_forking;

		LpHandlePtr _hangupHandle;

		LpHandlePtr _playStoppedHandle;

		LpHandlePair _imsSessionHandlerPair;
	};


	typedef shared_ptr<ImsSession> ImsSessionPtr;


}


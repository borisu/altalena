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

namespace ivrworx
{

	typedef 

	int ImsHandle;

	typedef map<int,char> 
	DtmfMap;

	class ImsSession :
		public ActiveObject
	{
	public:

		ImsSession (IN ScopedForking &forking);

		virtual ~ImsSession (void);

		virtual ApiErrorCode	AllocateIMSConnection();

		virtual ApiErrorCode	AllocateIMSConnection(
			IN CnxInfo remote_end, 
			IN MediaFormat codec);

		virtual ApiErrorCode	ModifyConnection(
			IN CnxInfo remote_end, 
			IN MediaFormat codec);

		virtual ApiErrorCode	StopPlay();

		virtual ApiErrorCode PlayFile(
			IN const string &file_name,
			IN BOOL sync = FALSE,
			IN BOOL loop = FALSE,
			IN BOOL provisional = FALSE);

		virtual ApiErrorCode WaitForDtmf(
			OUT int &dtmf, 
			IN Time timeout);

		virtual ApiErrorCode SendDtmf(char dtmf);
		
		virtual void TearDown();

		virtual CnxInfo ImsMediaData() const;

		virtual void ImsMediaData(IN CnxInfo val);

		virtual void UponActiveObjectEvent(IwMessagePtr ptr);

		virtual void InterruptWithHangup();

		virtual const string& GetDtmfString();

		virtual void ClearDtmfs();

	private:

		ApiErrorCode	SyncStreamFile();

		ImsHandle _imsSessionHandle;

		CnxInfo _imsMediaData;

		ScopedForking &_forking;

		LpHandlePtr _rfc2833DtmfHandle;

		LpHandlePtr _hangupHandle;

		LpHandlePair _imsSessionHandlerPair;

		string _dtmf;

		DtmfMap _dtmfMap;
	};

	typedef shared_ptr<ImsSession> ImsSessionPtr;


}


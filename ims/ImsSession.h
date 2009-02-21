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
	int ImsHandleId;

	class ImsSession
	{
	public:

		ImsSession (IN LightweightProcess &facade);

		virtual ~ImsSession (void);

		IxApiErrorCode	AllocateIMSConnection(
			IN CnxInfo remote_end, 
			IN MediaFormat codec);

		virtual IxApiErrorCode PlayFile(
			IN const wstring &file_name,
			IN BOOL sync = FALSE,
			IN BOOL loop = FALSE,
			IN BOOL provisional = FALSE);

		virtual void TearDown();

		virtual CnxInfo ImsMediaData() const;

		virtual void ImsMediaData(IN CnxInfo val);

	private:

		IxApiErrorCode	SyncStreamFile();

		CcuConnectionId _imsSessionHandle;

		CnxInfo _imsMediaData;

		LightweightProcess &_facade;
	};

	typedef shared_ptr<ImsSession> ImsSessionPtr;


}


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

#include "Ccu.h"
#include "LightweightProcess.h"

using namespace boost;

class ImsSession
{
public:

	ImsSession (
		IN LightweightProcess &facade);

	virtual ~ImsSession (void);

	CcuApiErrorCode	AllocateIMSConnection(
		IN CcuMediaData remote_end, 
		IN const wstring &file_name);

	virtual CcuApiErrorCode PlayFile(
		IN CcuMediaData destination, 
		IN const wstring &file_name);

	CcuMediaData ImsMediaData() const;

	void ImsMediaData(IN CcuMediaData val);

private:

	

	CcuApiErrorCode	SyncStreamFile();

	CcuConnectionId _imsSessionHandle;

	CcuMediaData _imsMediaData;

	LightweightProcess &_facade;
};

typedef shared_ptr<ImsSession> ImsSessionPtr;

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

#include "PortManager.h"
#include "RTPConnection.h"
#include "ccu.h"

typedef 
int ImsHandleId;

class StreamingObject : public UIDOwner
{
public:
	StreamingObject(
		IN const CnxInfo &media_data, 
		IN const wstring &file_name, 
		IN ImsHandleId handle_id);

	virtual ~StreamingObject(void);

	virtual CcuApiErrorCode Process();

	CcuApiErrorCode Init(PortManager &portManager);

	CcuApiErrorCode Close(PortManager &portManager);

	ImsHandleId ImsHandle();

	int Port() const;

	CnxInfo RemoteMediaData() const;

	void RemoteMediaData(CnxInfo val);

private:

	CnxInfo  _remoteMediaData;
	
	wstring _fileName;

	ImsHandleId _handleId;

	RTPSession _rtpSession;

	MIPTime _interval; 

	MIPAverageTimer _timer;

	MIPComponentChain _chain;

	MIPWAVInput _sndFileInput;

	MIPSamplingRateConverter _sampConv;

	MIPSampleEncoder _sampEnc;

	MIPULawEncoder _uLawEnc;

	MIPRTPULawEncoder _rtpEnc;

	MIPRTPComponent _rtpComp;

	int _port;

	rtp_int64_t _iteration;
	
	
};

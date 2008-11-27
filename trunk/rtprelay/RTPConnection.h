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
#include "UIDOwner.h"

using namespace std;
using namespace boost;

#pragma message("TODO: take this definitions into special file")

typedef ::real_t	rtp_real_t;
typedef ::uint64_t  rtp_uint64_t;
typedef ::uint32_t  rtp_uint32_t;
typedef ::uint8_t   rtp_uint8_t;
typedef ::int64_t   rtp_int64_t;


typedef shared_ptr<RTPPacket> RTPPacketPtr;

struct RtpReceiveMsg;

typedef list<RtpReceiveMsg> RtpPacketsList;

struct RtpReceiveMsg
{

	RtpReceiveMsg(RTPPacket *p);

	RtpReceiveMsg(const RtpReceiveMsg &p);

	RTPPacketPtr pPack;

	const rtp_uint8_t *pCName;

	MIPTime jitter;

	rtp_real_t timestampUnit;

	rtp_uint64_t sourceID;

	MIPTime m_timingInfWallclock;

	rtp_uint32_t m_timingInfTimestamp;

	bool m_timingInfoSet;

	void TimingInfo(MIPTime t, rtp_uint32_t timestamp);

	bool TimingInfo(MIPTime *t, rtp_uint32_t *timestamp);

};


class RTPConnection:
	public UIDOwner
{
	
public:

	RTPConnection(UINT port);

	virtual ~RTPConnection(void);

	virtual CcuApiErrorCode Init(); 

	virtual CcuApiErrorCode AddDestination(IN CcuMediaData &data);

	virtual CcuApiErrorCode SetDestination(IN CcuMediaData &data);

	virtual void Destroy();

	virtual void Poll(OUT RtpPacketsList &packetsList, IN size_t overflow);

	virtual void Send(IN RtpPacketsList &packetsList);

	virtual rtp_uint64_t SourceID(IN const RTPPacket *pPack, IN const RTPSourceData *pSourceData) const
	{
		return (rtp_uint64_t)(pPack->GetSSRC());
	}

	virtual int ConnectionId();

	UINT Port();

	CcuMediaDataList& DestinationsList();

private:

	virtual CcuApiErrorCode Send(RTPPacket *packet);

	int _localPort;

	CcuMediaDataList _destinationsList;
	
	RTPSession _rtpSession;

};

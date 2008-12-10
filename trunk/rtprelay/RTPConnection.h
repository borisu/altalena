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
#include "RelayMemoryManager.h"
#include "AsyncIocpRTPUDPv4Transmitter.h"

using namespace std;
using namespace boost;

typedef ::real_t	rtp_real_t;
typedef ::uint64_t  rtp_uint64_t;
typedef ::uint32_t  rtp_uint32_t;
typedef ::uint8_t   rtp_uint8_t;
typedef ::int64_t   rtp_int64_t;



struct RtpReceiveMsg :
	public boost::noncopyable, public RTPMemoryObject
{

	RtpReceiveMsg(RTPPacket *p, RelayMemoryManager *mngr = NULL);

	RtpReceiveMsg(const RtpReceiveMsg &p);

	~RtpReceiveMsg();

	RTPPacket *rtp_packet;

	const rtp_uint8_t *cname;

	MIPTime jitter;

	rtp_real_t timestamp_unit;

	rtp_uint64_t source_id;

	MIPTime timing_info_wallclock;

	rtp_uint32_t timing_info_timestamp;

	bool timing_info_set;

	void TimingInfo(MIPTime t, rtp_uint32_t timestamp);

	bool TimingInfo(MIPTime *t, rtp_uint32_t *timestamp);

};

typedef list<RtpReceiveMsg*> RtpPacketsList;


class RTPConnection:
	public UIDOwner
{
	
public:

	RTPConnection(UINT port, RelayMemoryManager *mngr = NULL);

	virtual ~RTPConnection(void);

	virtual CcuApiErrorCode Init(HANDLE iocpHandle);

	virtual CcuApiErrorCode AddDestination(IN CnxInfo &data);

	virtual CcuApiErrorCode SetDestination(IN CnxInfo &data);

	virtual void Destroy();

	virtual CcuApiErrorCode IssueAsyncIoReq(IN BOOL rtp);

	virtual CcuApiErrorCode Poll(OUT RtpPacketsList &packetsList, IN size_t overflow, IN BOOL relay_mode, IN RtpOverlapped *ovlap);

	virtual void AsyncRelayRtpPacket(IN RtpPacketsList &packetsList, bool releasePacket);

	virtual rtp_uint64_t SourceID(IN const RTPPacket *pPack, IN const RTPSourceData *pSourceData) const;

	virtual int ConnectionId();

	UINT Port();

	CcuMediaDataList& DestinationsList();

private:

	virtual CcuApiErrorCode AsyncRelayRtpPacket(RTPPacket *packet);

	RelayMemoryManager *GetMemoryManager();

	AsyncIocpRTPUDPv4Transmitter *_iocpAtrans;

	int _localPort;

	CcuMediaDataList _destinationsList;
	
	RTPSession _rtpSession;

	unsigned int _previousTimestamp;

	RelayMemoryManager *_memMngr;


};

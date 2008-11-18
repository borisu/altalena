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

	virtual CcuApiErrorCode AddDestination(IN long ipaddr, IN UINT port);

	virtual CcuApiErrorCode SetDestination(IN long ipaddr, IN UINT port);

	virtual void Destroy();

	virtual void Poll(RtpPacketsList &packetsList, size_t overflow);

	virtual void Send(RtpPacketsList &packetsList);

	virtual rtp_uint64_t SourceID(const RTPPacket *pPack, const RTPSourceData *pSourceData) const
	{
		return (rtp_uint64_t)(pPack->GetSSRC());
	}

	virtual int ConnectionId();

	UINT Port() const;

private:

	virtual CcuApiErrorCode Send(RTPPacket *packet);

	UINT _port;
	
	RTPSession _rtpSession;

};

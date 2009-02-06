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

#include "StdAfx.h"
#include "CcuLogger.h"
#include "RTPConnection.h"
#include "AsyncIocpRTPUDPv4Transmitter.h"


#define CCU_MAX_RTP_PACKET_SIZE			2048
#define CCU_DEFAULT_RTP_SAMPLING_RATE	8000

#define RTPMEM_TYPE_RTP_MSG_RECEIVE		5000
#define RTPMEM_TYPE_RTP_MSG_OVERLAPPED	5001
#define RTPMEM_TYPE_RTP_MSG_WSABUF		5002


RtpReceiveMsg::RtpReceiveMsg(RTPPacket *rtpPacket, RelayMemoryManager *memMngr):
RTPMemoryObject(memMngr),
rtp_packet(rtpPacket),
cname(NULL),
jitter(0),
timestamp_unit(0),
source_id (IX_UNDEFINED),
timing_info_wallclock (IX_UNDEFINED),
timing_info_timestamp (IX_UNDEFINED),
timing_info_set(false)
{

}

void 
RtpReceiveMsg::TimingInfo(MIPTime t, rtp_uint32_t timestamp)
{
	timing_info_wallclock = t; 
	timing_info_timestamp = timestamp; 
	timing_info_set = true; 
}

bool 
RtpReceiveMsg::TimingInfo(MIPTime *t, rtp_uint32_t *timestamp) 
{ 
	if (!timing_info_set) 
	{
		return false; 
	};

	*t = timing_info_wallclock; 
	*timestamp = timing_info_timestamp;

	return true; 
}

RtpReceiveMsg::~RtpReceiveMsg()
{
	RTPDelete(rtp_packet,GetMemoryManager());
}


RTPConnection::RTPConnection(UINT localPort, RelayMemoryManager *mngr ):
_localPort(localPort),
_rtpSession(mngr),
_previousTimestamp(IX_UNDEFINED),
_memMngr(mngr)
{
	
}

rtp_uint64_t 
RTPConnection::SourceID(IN const RTPPacket *pPack, IN const RTPSourceData *pSourceData) const
{
	return (rtp_uint64_t)(pPack->GetSSRC());
}

int
RTPConnection::ConnectionId()
{
	return this->GetObjectUid();
}


IxApiErrorCode 
RTPConnection::Init(HANDLE iocpHandle)
{
	
	
	RTPUDPv4TransmissionParams transmissionParams;
	RTPSessionParams sessionParams;
	
	transmissionParams.SetPortbase(_localPort);
	sessionParams.SetOwnTimestampUnit(1.0/((double)CCU_DEFAULT_RTP_SAMPLING_RATE));
	sessionParams.SetMaximumPacketSize(CCU_MAX_RTP_PACKET_SIZE);
	sessionParams.SetAcceptOwnPackets(true);
	sessionParams.SetUsePollThread(false);

	// transmitter will be deleted by RtpSession object
	_iocpAtrans = new AsyncIocpRTPUDPv4Transmitter(GetMemoryManager(), iocpHandle);
	
	int status = _rtpSession.Create(
			sessionParams,
			&transmissionParams,
			_iocpAtrans);

	if (status < 0)
	{
		LogWarn("Cannot create RTP session on port=[" << _localPort << "] API error=[" <<  status  <<"] check rtperrors.h for description.");
		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;
	
}

IxApiErrorCode 
RTPConnection::IssueAsyncIoReq(BOOL rtp)
{
	if (_iocpAtrans == NULL)
	{
		LogWarn("Tried to issue request on empty transmitter. Dear developer have called the 'Init' function?");
		return CCU_API_FAILURE;
	}

	if (rtp == TRUE)
	{
		// rtp:
		RtpOverlapped *rtp_ovlap = RTPNew(GetMemoryManager(), RTPMEM_TYPE_RTP_MSG_OVERLAPPED) RtpOverlapped; 
		::SecureZeroMemory(rtp_ovlap , sizeof(RtpOverlapped));
		rtp_ovlap->ctx = this;
		rtp_ovlap->rtp = TRUE;
		rtp_ovlap->opcode = OPCODE_READ;

		if (_iocpAtrans->IssueAsyncRead(TRUE, &rtp_ovlap->oOverlap) < 0)
		{
			LogWarn("Cannot issue read request for rtp socket");
			RTPDelete(rtp_ovlap , GetMemoryManager());
			return CCU_API_FAILURE;
		}
	} 
	else
	{
		// rtcp:
		RtpOverlapped *rtcp_ovlap = RTPNew(GetMemoryManager(), RTPMEM_TYPE_RTP_MSG_OVERLAPPED) RtpOverlapped; 
		::SecureZeroMemory(rtcp_ovlap , sizeof(RtpOverlapped));
		rtcp_ovlap->ctx = this;
		rtcp_ovlap->rtp = FALSE;
		rtcp_ovlap->opcode = OPCODE_READ;

		if (_iocpAtrans->IssueAsyncRead(FALSE, &rtcp_ovlap->oOverlap) < 0)
		{
			LogWarn("Cannot issue read request for rtcp socket");
			RTPDelete(rtcp_ovlap , GetMemoryManager());
			return CCU_API_FAILURE;
		}

	}
	
	
	return CCU_API_SUCCESS;
	
}

IxApiErrorCode 
RTPConnection::AddDestination(IN CnxInfo &data)
{
	
	int status = _rtpSession.AddDestination(
		RTPIPv4Address(
		data.iaddr_ho(),
		data.port_ho()));

	if (status < 0)
	{
		LogWarn("Cannot add destination to RTP session on port=[" << _localPort << "] error=[" <<  status  <<"] dest ip=[" << data.ipporttows() << "]" );
		return CCU_API_FAILURE;
	}


	_destinationsList.push_back(data);

	
	return CCU_API_SUCCESS;

}

UINT 
RTPConnection::Port()
{ 
	return _localPort; 
}

void
RTPConnection::Destroy()
{
	_rtpSession.BYEDestroy(RTPTime(0),NULL,0);
	
}

IxApiErrorCode
RTPConnection::Poll(OUT RtpPacketsList &packetsList, IN size_t overflow, IN BOOL relayMode, RtpOverlapped *ovlap)
{
	
	//
	// The code of the function was mostly cut and paste
	// from 'bool MIPRTPComponent::processNewPackets(int64_t iteration)'.
	// See it for reference.
	//
#pragma warning (suppress : 4800)
	int res = _rtpSession.AsyncPoll(relayMode, ovlap->rtp, &ovlap->oOverlap);
	if ( res != 0)
	{
		LogWarn("Error polling connection stack res=[" << res << "] description=[" << 
			StringToWString(RTPGetErrorString(res)) << "]");
		return CCU_API_FAILURE;
	}

	//
	// start it all over
	//
	IssueAsyncIoReq(ovlap->rtp);

	//
	// Mostly for synchronization purposes,
	// Not so relevant in our single threaded
	// model, but I left it in any case.
	//
	res = _rtpSession.BeginDataAccess();
	if ( res != 0)
	{
		LogWarn("Cannot begin data access res=[" << res << "] description=[" << 
			StringToWString(RTPGetErrorString(res)) << "]");
		return CCU_API_FAILURE;
	}


	if (_rtpSession.GotoFirstSourceWithData())
	{
		do
		{
			RTPSourceData *srcData = _rtpSession.GetCurrentSourceInfo();

			
			//
			// Get common data of the source 
			// (Not particular to certain RTP packet).
			//
			size_t				cname_length		= 0;
			const rtp_uint8_t	*cname				= srcData->SDES_GetCNAME(&cname_length);
			rtp_uint32_t		jitter_samples		= srcData->INF_GetJitter();
			rtp_real_t			jitter_seconds		= 0;
			rtp_real_t			ts_unit				= 0;
			rtp_real_t			ts_unit_ustimation	= 0;
			bool				setTimingInfo		= false;
			rtp_uint32_t		timingInfTimestamp	= 0;
			MIPTime				timingInfWallclock(0);

			if ((ts_unit = (rtp_real_t)srcData->GetTimestampUnit()) > 0)
			{
				jitter_seconds = (rtp_real_t)jitter_samples*ts_unit;
			}
			else
			{
				if ((ts_unit_ustimation = (rtp_real_t)srcData->INF_GetEstimatedTimestampUnit()) > 0)
				{
					jitter_seconds = (rtp_real_t)jitter_samples*ts_unit_ustimation;
				}
			}

			if (srcData->SR_HasInfo())
			{
				RTPNTPTime ntpTime = srcData->SR_GetNTPTimestamp();

				if (!(ntpTime.GetMSW() == 0 && ntpTime.GetLSW() == 0))
				{
					setTimingInfo = true;
					RTPTime rtpTime(ntpTime);
					timingInfWallclock = MIPTime(rtpTime.GetSeconds(),rtpTime.GetMicroSeconds());
					timingInfTimestamp = srcData->SR_GetRTPTimestamp();
				}
			}

			RTPPacket *pPack = NULL;
			while ((pPack = _rtpSession.GetNextPacket()) != 0)
			{
				// overflow => discard packet
				if (packetsList.size() > overflow)
				{
					RTPDelete(pPack, GetMemoryManager());
					continue;
				}

				RtpReceiveMsg *rtpMsg = 
					RTPNew(GetMemoryManager(), RTPMEM_TYPE_RTP_MSG_RECEIVE) RtpReceiveMsg(pPack,GetMemoryManager()); 

				// some strange calculations
				// which I don't fully understand

				rtpMsg->jitter = MIPTime(jitter_seconds);

				if (ts_unit > 0)
					rtpMsg->timestamp_unit = ts_unit;

				if (setTimingInfo)
					rtpMsg->TimingInfo(timingInfWallclock, timingInfTimestamp);

				rtpMsg->source_id = SourceID(pPack, srcData);

				packetsList.push_back(rtpMsg);
			}

		} while (_rtpSession.GotoNextSourceWithData());
	}

	


	_rtpSession.EndDataAccess();

	return CCU_API_SUCCESS;

}

IxApiErrorCode
RTPConnection::SetDestination(IN CnxInfo &data)
{
	_rtpSession.ClearDestinations();

	int status = 	
		_rtpSession.AddDestination(RTPIPv4Address(data.iaddr_ho(), data.port_ho()));

	if (status < 0)
	{
		LogWarn("Cannot add destination to RTP error=[" <<  status  <<"] dest ip=[" << data.ipporttows() <<  "]" );
		return CCU_API_FAILURE;
	}

	_destinationsList.clear();
	_destinationsList.push_back(data);

	return CCU_API_SUCCESS;

}

CcuMediaDataList& 
RTPConnection::DestinationsList()
{

	return _destinationsList;
}




void
RTPConnection::AsyncRelayRtpPacket(RtpPacketsList &packetsList, bool releasePacket)
{

	for(RtpPacketsList::iterator i = packetsList.begin(); i!=packetsList.end(); i++)
	{
		RtpReceiveMsg *msg = (*i);
		AsyncRelayRtpPacket(msg->rtp_packet);

		if (!releasePacket)
		{
			continue;
		}

		i = packetsList.erase(i);
		RTPDelete(msg, _memMngr);

		if ( i == packetsList.end())
		{
			break;
		}
	}

}

	
IxApiErrorCode
RTPConnection::AsyncRelayRtpPacket(RTPPacket *packet)
{

	RtpOverlapped *rtp_ovlap = 
		RTPNew(_memMngr, RTPMEM_TYPE_RTP_MSG_OVERLAPPED) RtpOverlapped; 
	if ( rtp_ovlap == 0)
	{
		LogCrit("Out Of Memory");
		return CCU_API_FAILURE;
	}

	::SecureZeroMemory(rtp_ovlap , sizeof(RtpOverlapped));
	rtp_ovlap->ctx = this;
	rtp_ovlap->rtp = TRUE;
	rtp_ovlap->opcode = OPCODE_WRITE;

	LPWSABUF  sendbuf = 
		RTPNew(GetMemoryManager(),RTPMEM_TYPE_RTP_MSG_WSABUF) WSABUF; 
	if ( sendbuf == 0)
	{
		RTPDelete(rtp_ovlap,GetMemoryManager());
		LogCrit("Out Of Memory");
		return CCU_API_FAILURE;
	}

	int len = packet->GetRawPacket()->GetDataLength();
	char *datacopy = 
		RTPNew(GetMemoryManager(), RTPMEM_TYPE_BUFFER_RECEIVEDRTPPACKET) char[len];

	if (datacopy == 0)
	{
		RTPDelete(rtp_ovlap,GetMemoryManager());
		RTPDelete(sendbuf,GetMemoryManager());
		LogCrit("Out Of Memory");
		return CCU_API_FAILURE;
	}

	::memcpy(
		datacopy,
		packet->GetRawPacket()->GetData(),
		len);

	sendbuf->buf = datacopy;
	sendbuf->len = packet->GetRawPacket()->GetDataLength();

	int status = _rtpSession.AsyncRelayRtpPacket(sendbuf,&rtp_ovlap->oOverlap);
	if (status < 0)
	{
		
		RTPDelete(datacopy,GetMemoryManager());
		RTPDelete(rtp_ovlap,GetMemoryManager());
		RTPDelete(sendbuf,GetMemoryManager());
		
		return CCU_API_FAILURE;
	}

	rtp_ovlap->send_buf = sendbuf;

	return CCU_API_SUCCESS;
}

RelayMemoryManager*
RTPConnection::GetMemoryManager()
{
	return _memMngr;
}

RTPConnection::~RTPConnection(void)
{

		
}

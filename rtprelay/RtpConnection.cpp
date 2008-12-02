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


#define CCU_MAX_RTP_PACKET_SIZE			64000
#define CCU_DEFAULT_RTP_SAMPLING_RATE	8000

#define RTPMEM_TYPE_RTP_MSG_RECEIVE		5000


RtpReceiveMsg::RtpReceiveMsg(RTPPacket *rtpPacket, RelayMemoryManager *memMngr):
RTPMemoryObject(memMngr),
rtp_packet(rtpPacket),
cname(NULL),
jitter(0),
timestamp_unit(0),
source_id (CCU_UNDEFINED),
timing_info_wallclock (CCU_UNDEFINED),
timing_info_timestamp (CCU_UNDEFINED),
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
_previousTimestamp(CCU_UNDEFINED),
_memMngr(mngr)
{
	
}


int
RTPConnection::ConnectionId()
{
	return this->GetObjectUid();
}


CcuApiErrorCode 
RTPConnection::Init()
{
	
	
	RTPUDPv4TransmissionParams transmissionParams;
	RTPSessionParams sessionParams;
	


	transmissionParams.SetPortbase(_localPort);
	sessionParams.SetOwnTimestampUnit(1.0/((double)CCU_DEFAULT_RTP_SAMPLING_RATE));
	sessionParams.SetMaximumPacketSize(CCU_MAX_RTP_PACKET_SIZE);
	sessionParams.SetAcceptOwnPackets(true);
	sessionParams.SetUsePollThread(false);
	


	int status = _rtpSession.Create(sessionParams,&transmissionParams,RTPTransmitter::IPv4UDPProto);
	if (status < 0)
	{
		LogWarn("Cannot create RTP session on port=[" << _localPort << "] API error=[" <<  status  <<"] check rtperrors.h for description.");
		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;
	
}

CcuApiErrorCode 
RTPConnection::AddDestination(IN CnxInfo &data)
{
	
	int status = _rtpSession.AddDestination(
		RTPIPv4Address(
		data.iaddr_ho(),
		data.port_ho()));

	if (status < 0)
	{
		LogWarn("Cannot add destination to RTP session on port=[" << _localPort << "] error=[" <<  status  <<"] dest ip=[" << data.ipporttos() << "]" );
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

CcuApiErrorCode
RTPConnection::Poll(OUT RtpPacketsList &packetsList, IN size_t overflow, IN bool relayMode)
{
	
	//
	// The code of the function was mostly cut and paste
	// from 'bool MIPRTPComponent::processNewPackets(int64_t iteration)'.
	// See it for reference.
	//
	int res = _rtpSession.Poll(relayMode);
	if ( res != 0)
	{
		LogWarn("Error polling connection stack res=[" << res << "] description=[" << RTPGetErrorString(res) << "]");
		return CCU_API_FAILURE;
	}

	//
	// Mostly for synchronization purposes,
	// Not so relevant in our single threaded
	// model, but I left it in any case.
	//
	res = _rtpSession.BeginDataAccess();
	if ( res != 0)
	{
		LogWarn("Cannot begin data access res=[" << res << "] description=[" << RTPGetErrorString(res) << "]");
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
					RTPDelete(pPack, _memMngr);
					continue;
				}

				RtpReceiveMsg *rtpMsg = RTPNew(_memMngr, RTPMEM_TYPE_RTP_MSG_RECEIVE) RtpReceiveMsg(pPack,_memMngr); 

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

CcuApiErrorCode
RTPConnection::SetDestination(IN CnxInfo &data)
{
	_rtpSession.ClearDestinations();

	int status = 	
		_rtpSession.AddDestination(RTPIPv4Address(data.iaddr_ho(), data.port_ho()));

	if (status < 0)
	{
		LogWarn("Cannot add destination to RTP error=[" <<  status  <<"] dest ip=[" << data.ipporttos() <<  "]" );
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
RTPConnection::Send(RtpPacketsList &packetsList, bool releasePacket)
{

	for(RtpPacketsList::iterator i = packetsList.begin(); i!=packetsList.end(); i++)
	{
		RtpReceiveMsg *msg = (*i);
		Send(msg->rtp_packet);

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

CcuApiErrorCode
RTPConnection::Send(RTPPacket *packet)
{

	int status = _rtpSession.RelayPacket(packet->GetRawPacket());


	if (status < 0)
	{
		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;
}


RTPConnection::~RTPConnection(void)
{

		
}

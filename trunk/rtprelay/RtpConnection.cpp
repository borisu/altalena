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


#define CCU_MAX_RTP_PACKET_SIZE		  64000
#define CCU_DEFAULT_RTP_SAMPLING_RATE 8000



RtpReceiveMsg::RtpReceiveMsg(RTPPacket *p):
pPack(p),
pCName(NULL),
jitter(0),
timestampUnit(0),
sourceID (-1),
m_timingInfWallclock (-1),
m_timingInfTimestamp (-1),
m_timingInfoSet (false)
{

}

RtpReceiveMsg::RtpReceiveMsg(const RtpReceiveMsg &p)
{
	pPack = p.pPack;
	pCName = p.pCName;
	jitter = p.jitter;
	timestampUnit = p.timestampUnit;
	sourceID = p.sourceID;
	m_timingInfWallclock = p.m_timingInfWallclock;
	m_timingInfTimestamp = p.m_timingInfTimestamp;
	m_timingInfoSet = p.m_timingInfoSet;
}

void 
RtpReceiveMsg::TimingInfo(MIPTime t, rtp_uint32_t timestamp)
{
	m_timingInfWallclock = t; m_timingInfTimestamp = timestamp; m_timingInfoSet = true; 
}

bool 
RtpReceiveMsg::TimingInfo(MIPTime *t, rtp_uint32_t *timestamp) 
{ 
	if (!m_timingInfoSet) return false; *t = m_timingInfWallclock; *timestamp = m_timingInfTimestamp; return true; 
}


RTPConnection::RTPConnection(UINT localPort):
_localPort(localPort),
_rtpSession(NULL)
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
	


	int status = _rtpSession.Create(sessionParams,&transmissionParams);
	if (status < 0)
	{
		LogWarn("Cannot create RTP session on port=[" << _localPort << "] API error=[" <<  status  <<"] check rtperrors.h for description.");
		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;
	
}

CcuApiErrorCode 
RTPConnection::AddDestination(long remoteIpAddr, UINT remotePort)
{
	
	int status = _rtpSession.AddDestination(
		RTPIPv4Address(ntohl(remoteIpAddr),remotePort));

	if (status < 0)
	{
		LogWarn("Cannot add destination to RTP session on port=[" << _localPort << "] error=[" <<  status  <<"] dest ip=[" << remoteIpAddr << "] dest port=[" << remotePort << "]" );
		return CCU_API_FAILURE;
	}

	_destinationsList.push_back(
		CcuMediaData(remoteIpAddr,remotePort));

	
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

void
RTPConnection::Poll(RtpPacketsList &packetsList, size_t overflow)
{
	
	int res = _rtpSession.Poll();
	if ( res != 0)
	{
		LogWarn("Error polling connection stack res=[" << res << "] description=[" << RTPGetErrorString(res) << "]");
		return;
	}

	res = _rtpSession.BeginDataAccess();
	if ( res != 0)
	{
		LogWarn("Cannot begin data access res=[" << res << "] description=[" << RTPGetErrorString(res) << "]");
		return;
	}


	if (_rtpSession.GotoFirstSourceWithData())
	{
		do
		{
			RTPSourceData *srcData = _rtpSession.GetCurrentSourceInfo();

			size_t cnameLength;
			const rtp_uint8_t *pCName = srcData->SDES_GetCNAME(&cnameLength);
			rtp_uint32_t jitterSamples = srcData->INF_GetJitter();
			rtp_real_t jitterSeconds = 0;
			rtp_real_t tsUnit;
			rtp_real_t tsUnitEstimation;
			bool setTimingInfo = false;
			rtp_uint32_t timingInfTimestamp = 0;
			MIPTime timingInfWallclock(0);

			if ((tsUnit = (rtp_real_t)srcData->GetTimestampUnit()) > 0)
				jitterSeconds = (rtp_real_t)jitterSamples*tsUnit;
			else
			{
				if ((tsUnitEstimation = (rtp_real_t)srcData->INF_GetEstimatedTimestampUnit()) > 0)
					jitterSeconds = (rtp_real_t)jitterSamples*tsUnitEstimation;
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
				//
				// overflow => discard packet
				//
				if (packetsList.size() > overflow)
				{
					continue;
				}

				_ASSERT(pPack != NULL);
#ifndef DEBUG
				if (pPack == NULL)
				{
					continue;
				}
#endif
				
				RtpReceiveMsg rtpMsg(pPack); 

				rtpMsg.jitter = MIPTime(jitterSeconds);
				if (tsUnit > 0)
					rtpMsg.timestampUnit = tsUnit;
				if (setTimingInfo)
					rtpMsg.TimingInfo(timingInfWallclock, timingInfTimestamp);
				rtpMsg.sourceID = SourceID(pPack, srcData);

				

				packetsList.push_back(rtpMsg);
			}
		} while (_rtpSession.GotoNextSourceWithData());
	}
	_rtpSession.EndDataAccess();

}

CcuApiErrorCode
RTPConnection::SetDestination(IN long remoteIpAddr, IN UINT remotePort)
{
	_rtpSession.ClearDestinations();

	int status = 	
		_rtpSession.AddDestination(RTPIPv4Address(ntohl(remoteIpAddr),remotePort));

	if (status < 0)
	{
		LogWarn("Cannot add destination to RTP error=[" <<  status  <<"] dest ip=[" << remoteIpAddr << "] dest port=[" << remotePort << "]" );
		return CCU_API_FAILURE;
	}

	_destinationsList.clear();
	_destinationsList.push_back(
		CcuMediaData(remoteIpAddr,remotePort));

	return CCU_API_SUCCESS;

}

const CcuMediaDataList& 
RTPConnection::DestinationsList()
{

	return _destinationsList;
}




void
RTPConnection::Send(RtpPacketsList &packetsList)
{

	for(RtpPacketsList::iterator i = packetsList.begin(); i!=packetsList.end(); i++)
	{
		RtpReceiveMsg *msg = &(*i);
		Send(msg->pPack.get());
	}
}

CcuApiErrorCode
RTPConnection::Send(RTPPacket *packet)
{

	int status = _rtpSession.SendPacket(
		packet->GetPayloadData(),
		packet->GetPayloadLength(),
		packet->GetPayloadType(),
		packet->HasMarker(),
		0);

	if (status < 0)
	{
		return CCU_API_FAILURE;
	}

	return CCU_API_SUCCESS;
}


RTPConnection::~RTPConnection(void)
{

		
}

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

#include "rtpudpv4transmitter.h"
#include "RtpOverlapped.h"


class AsyncIocpRTPUDPv4Transmitter :
	public RTPUDPv4Transmitter
{
public:

	AsyncIocpRTPUDPv4Transmitter(
		IN RTPMemoryManager *mgr, 
		IN HANDLE iocpHandle);

	~AsyncIocpRTPUDPv4Transmitter(void);

	virtual int Create(
		IN size_t maximumpacketsize, 
		IN const RTPTransmissionParams *transparams);

	virtual int AsyncPoll(
		IN BOOL rtp, 
		IN LPWSAOVERLAPPED ovlap);

	virtual int AsyncSendRTPData(
		IN LPWSABUF buf, 
		IN LPWSAOVERLAPPED ovlap);	

	virtual int AsyncSendRTCPData(
		IN LPWSABUF buf, 
		IN LPWSAOVERLAPPED ovlap);

	virtual int	IssueAsyncRead(
		IN BOOL rtp, 
		IN LPWSAOVERLAPPED lpOverlapped);

private:

	HANDLE _iocpHandle;

	char _rtpReadPacketBuffer[RTPUDPV4TRANS_MAXPACKSIZE];

	char _rtcpReadPacketBuffer[RTPUDPV4TRANS_MAXPACKSIZE];

	WSABUF _rtpReadWsaBufStruct;

	WSABUF _rtcpReadWsaBufStruct;

	struct sockaddr _rtpFrom;

	struct sockaddr _rtcpFrom;

	bool _ioRtpReadOpPending;

	bool _ioRtcpReadOpPending;

};

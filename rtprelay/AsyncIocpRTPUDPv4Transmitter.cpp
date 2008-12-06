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
#include "Ccu.h"
#include "AsyncIocpRTPUDPv4Transmitter.h"


#define SIO_UDP_CONNRESET   _WSAIOW(IOC_VENDOR,12) 

typedef
std::list<::uint32_t> IpAddrList; 


int AsyncIocpRTPUDPv4Transmitter::_keyCounter = 0; 

int AsyncIocpRTPUDPv4Transmitter::GenerateNewIocpKey()
{
	return _keyCounter++;
}

AsyncIocpRTPUDPv4Transmitter::AsyncIocpRTPUDPv4Transmitter(
	IN RTPMemoryManager *mgr, 
	IN HANDLE iocpHandle)
:RTPUDPv4Transmitter(mgr),
_iocpHandle(iocpHandle),
_rtpIocpKey(CCU_UNDEFINED),
_rtcpIocpKey(CCU_UNDEFINED),
_lastRtpReadResult(FALSE),
_lastRtcpReadResult(FALSE)
{
	::SecureZeroMemory(&_rtpFrom, sizeof(_rtpFrom));
	::SecureZeroMemory(&_rtcpFrom, sizeof(_rtcpFrom));
	
	::SecureZeroMemory(&_rtpWsaBufStruct, sizeof(_rtpWsaBufStruct));
	::SecureZeroMemory(_rtpPacketBuffer, RTPUDPV4TRANS_MAXPACKSIZE);
	_rtpWsaBufStruct.buf = _rtpPacketBuffer;
	_rtpWsaBufStruct.len = RTPUDPV4TRANS_MAXPACKSIZE;

	::SecureZeroMemory(&_rtcpWsaBufStruct, sizeof(_rtcpWsaBufStruct));
	::SecureZeroMemory(_rtcpPacketBuffer, RTPUDPV4TRANS_MAXPACKSIZE);
	_rtcpWsaBufStruct.buf = _rtcpPacketBuffer;
	_rtpWsaBufStruct.len = RTPUDPV4TRANS_MAXPACKSIZE;
}


#pragma  region Create
int 
AsyncIocpRTPUDPv4Transmitter::Create(
							IN size_t maximumpacketsize,
							IN const RTPTransmissionParams *transparams)
{

#pragma TODO ("Unlike other transmitters this one is not multi threaded and built for Win32 API.")

	const RTPUDPv4TransmissionParams *params = NULL;
	const RTPUDPv4TransmissionParams defaultparams;

	struct sockaddr_in addr;
	::memset(&addr,0,sizeof(addr));

	int size	= 0;
	int status  = 0;

	// sanity check
	if (!init)
	{
		return ERR_RTP_UDPV4TRANS_NOTINIT;
	}

	if (created)
	{
		return ERR_RTP_UDPV4TRANS_ALREADYCREATED;
	};

	// Obtain transmission parameters
	if (transparams == NULL)
	{
		params = &defaultparams;
	}
	else
	{
		if (transparams->GetTransmissionProtocol() != RTPTransmitter::IPv4UDPProto)
		{
			return ERR_RTP_UDPV4TRANS_ILLEGALPARAMETERS;
		}
		params = (const RTPUDPv4TransmissionParams *)transparams;
	}

	// Check if portbase is even
	if (params->GetPortbase()%2 != 0)
	{
		return ERR_RTP_UDPV4TRANS_PORTBASENOTEVEN;
	}

	// create sockets
	// rtp:
	rtpsock = ::WSASocket(PF_INET, SOCK_DGRAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	if (rtpsock == INVALID_SOCKET)
	{
		LogSysError("::WSASocket");
		return ERR_RTP_UDPV4TRANS_CANTCREATESOCKET;
	}

	// rtcp:
	rtcpsock = ::WSASocket(PF_INET, SOCK_DGRAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	if (rtpsock == INVALID_SOCKET)
	{
		LogSysError("::WSASocket");
		return ERR_RTP_UDPV4TRANS_CANTCREATESOCKET;
	}

	// Associate sockets with io completion ports
	// rtp:
	_rtpIocpKey = GenerateNewIocpKey();
	if ( _iocpHandle != 
		::CreateIoCompletionPort((HANDLE)rtpsock, _iocpHandle, _rtpIocpKey, 0))
	{
		LogSysError("::CreateIoCompletionPort");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_SPECIFIEDSIZETOOBIG;
	}

	// rtcp
	_rtcpIocpKey = GenerateNewIocpKey();
	if ( _iocpHandle != 
		::CreateIoCompletionPort((HANDLE)rtpsock, _iocpHandle, _rtcpIocpKey, 0))
	{
		LogSysError("::CreateIoCompletionPort");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_SPECIFIEDSIZETOOBIG;
	}



	//rtp
	DWORD lpcbBytesReturned = 0;
	BOOL  report_icmp_not_reachable = FALSE;	
	status = ::WSAIoctl(
		rtpsock, 
		SIO_UDP_CONNRESET,
		&report_icmp_not_reachable,
		sizeof(report_icmp_not_reachable),
		NULL,
		0,
		&lpcbBytesReturned,
		NULL,
		NULL);
	if (status = SOCKET_ERROR)
	{
		LogSysError("::WSAIoctl");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTCREATESOCKET;
	}

	//rtcp
	status = ::WSAIoctl(
		rtcpsock, 
		SIO_UDP_CONNRESET,
		&report_icmp_not_reachable,
		sizeof(report_icmp_not_reachable),
		NULL,
		0,
		&lpcbBytesReturned,
		NULL,
		NULL);

	if (status = SOCKET_ERROR)
	{
		LogSysError("::WSAIoctl");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTCREATESOCKET;
	}
		

	// set socket buffer sizes
	size = params->GetRTPReceiveBuffer();
	if (::setsockopt(rtpsock,SOL_SOCKET,SO_RCVBUF,(const char *)&size,sizeof(int)) != 0)
	{
		LogSysError("::setsockopt");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTSETRTPRECEIVEBUF;
	}

	size = params->GetRTPSendBuffer();
	if (::setsockopt(rtpsock,SOL_SOCKET,SO_SNDBUF,(const char *)&size,sizeof(int)) != 0)
	{
		LogSysError("::setsockopt");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTSETRTPTRANSMITBUF;
	}

	size = params->GetRTCPReceiveBuffer();
	if (::setsockopt(rtcpsock,SOL_SOCKET,SO_RCVBUF,(const char *)&size,sizeof(int)) != 0)
	{
		LogSysError("::setsockopt");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTSETRTCPRECEIVEBUF;
	}

#pragma TODO ("Check if 0 send buffer improve socket performance")

	size = params->GetRTCPSendBuffer();
	if (::setsockopt(rtcpsock,SOL_SOCKET,SO_SNDBUF,(const char *)&size,sizeof(int)) != 0)
	{
		LogSysError("::setsockopt");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTSETRTCPRECEIVEBUF;
	}

	// bind sockets
	bindIP = params->GetBindIP();
	mcastifaceIP = params->GetMulticastInterfaceIP();

	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(params->GetPortbase());
	addr.sin_addr.s_addr = htonl(bindIP);
	if (::bind(rtpsock,(struct sockaddr *)&addr,sizeof(struct sockaddr_in)) != 0)
	{
		LogSysError("::bind");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTBINDRTPSOCKET;
	}


	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(params->GetPortbase()+1);
	addr.sin_addr.s_addr = htonl(bindIP);
	if (::bind(rtcpsock,(struct sockaddr *)&addr,sizeof(struct sockaddr_in)) != 0)
	{
		LogSysError("::bind");
		::closesocket(rtpsock);
		::closesocket(rtcpsock);
		return ERR_RTP_UDPV4TRANS_CANTBINDRTPSOCKET;
	}




	// Try to obtain local IP addresses
	localIPs = params->GetLocalIPList();
	if (localIPs.empty()) // User did not provide list of local IP addresses, calculate them
	{
		if ((status = CreateLocalIPList()) < 0)
		{
			::closesocket(rtpsock);
			::closesocket(rtcpsock);
			return status;
		}

		LogInfo("Found these local IP addresses:");
		for (IpAddrList::const_iterator  it = localIPs.begin() ; 
			it != localIPs.end() ; 
			it++)
		{
#pragma warning (suppress : 4244)
			RTPIPv4Address a(*it);
			LogDebug(StringToWString(a.GetAddressString()));;

		}

	}

#ifdef RTP_SUPPORT_IPV4MULTICAST
		if (SetMulticastTTL(params->GetMulticastTTL()))
			supportsmulticasting = true;
		else
			supportsmulticasting = false;
#else	// no multicast support enabled
		supportsmulticasting = false;
#endif	// RTP_SUPPORT_IPV4MULTICAST

		if ((status = CreateAbortDescriptors()) < 0)
		{
			::closesocket(rtpsock);
			::closesocket(rtcpsock);
			return status;
		}

		if (maximumpacketsize > RTPUDPV4TRANS_MAXPACKSIZE)
		{
			::closesocket(rtpsock);
			::closesocket(rtcpsock);
			DestroyAbortDescriptors();
			return ERR_RTP_UDPV4TRANS_SPECIFIEDSIZETOOBIG;
		}


		maxpacksize = maximumpacketsize;
		portbase = params->GetPortbase();
		multicastTTL = params->GetMulticastTTL();
		receivemode = RTPTransmitter::AcceptAll;

		localhostname = 0;
		localhostnamelength = 0;

		waitingfordata = false;
		created = true;
		
		return 0;
}

#pragma  endregion Create



int
AsyncIocpRTPUDPv4Transmitter::IssueAsyncRead(IN bool rtp)
{

	SOCKET s		             = rtp ? rtpsock : rtcpsock;
	LPWSABUF buf	             = rtp ? &_rtpWsaBufStruct : &_rtpWsaBufStruct;
	BOOL &read_res			     = rtp ? _lastRtpReadResult : _lastRtcpReadResult;
	struct sockaddr* src_addr    = rtp ? &_rtpFrom : &_rtcpFrom;
	LPWSAOVERLAPPED lpOverlapped = rtp ? &_rtpOverlapped : &_rtcpOverlapped;

	int src_addr_size =  sizeof(src_addr);
	int res = ::WSARecvFrom(
		s,			// A descriptor identifying a socket.
		buf,		// A pointer to an array of WSABUF structures.
		1,			// The number of WSABUF structures in the lpBuffers array.
		NULL,		// A pointer to the number of bytes received by this call if the WSARecvFrom operation completes immediately. If the lpOverlapped parameter is non-NULL, this parameter is optional and can be set to NULL.
		NULL,				// A pointer to flags used to modify the behavior of the WSARecvFrom function call.
		src_addr,			// An optional pointer to a buffer that will hold the source address upon the completion of the overlapped operation.
		&src_addr_size,		// A pointer to the size, in bytes, of the "from" buffer required only if lpFrom is specified.
		lpOverlapped,		// A pointer to a WSAOVERLAPPED structure (ignored for nonoverlapped sockets).
		NULL		// A pointer to the completion routine called when the WSARecvFrom operation has been completed (ignored for nonoverlapped sockets).
		);

	if ((SOCKET_ERROR != res) || 
		(::WSAGetLastError() == WSA_IO_PENDING))
	{
		read_res = TRUE;
		return ERR_RTP_UDPV4TRANS_CANNOTSTART_OVERLAPPPED;
	};

	read_res = FALSE;
	LogSysError("::WSARecvFrom");

	return 0;
}

int
AsyncIocpRTPUDPv4Transmitter::PollSocket(IN bool rtp)
{
	BOOL &read_res = rtp ? _lastRtpReadResult : _lastRtcpReadResult;
	if (read_res == FALSE)
	{
		return IssueAsyncRead(rtp);
	}

	SOCKET sock					 = rtp ? rtpsock : rtcpsock;
	LPWSAOVERLAPPED lpOverlapped = rtp ? &_rtpOverlapped : &_rtcpOverlapped;
	LPWSABUF buf				 = rtp ? &_rtpWsaBufStruct : &_rtpWsaBufStruct;
	struct sockaddr_in *src_addr = (sockaddr_in *) (rtp ? &_rtpFrom : &_rtcpFrom);


	RTPTime curtime = RTPTime::CurrentTime();
	

	DWORD recvlen = 0;
	DWORD flags	  = 0;

	BOOL res = ::WSAGetOverlappedResult(
		sock,			// A descriptor identifying the socket. This is the same socket that was specified when the overlapped operation was started by a call to WSARecv, WSARecvFrom, WSASend, WSASendTo, or WSAIoctl.
		lpOverlapped,   // A pointer to a WSAOVERLAPPED structure that was specified when the overlapped operation was started. This parameter must not be a NULL pointer.
		&recvlen,		// A pointer to a 32-bit variable that receives the number of bytes that were actually transferred by a send or receive operation, or by WSAIoctl. This parameter must not be a NULL pointer.
		FALSE,			// A flag that specifies whether the function should wait for the pending overlapped operation to complete. If TRUE, the function does not return until the operation has been completed. If FALSE and the operation is still pending, the function returns FALSE and the WSAGetLastError function returns WSA_IO_INCOMPLETE. The fWait parameter may be set to TRUE only if the overlapped operation selected the event-based completion notification.	
		&flags);		// A pointer to a 32-bit variable that will receive one or more flags that supplement the completion status. If the overlapped operation was initiated through WSARecv or WSARecvFrom, this parameter will contain the results value for lpFlags parameter. This parameter must not be a NULL pointer.

	if (res == FALSE || recvlen <= 0)	
	{
		LogSysError("::WSAGetOverlappedResult");
		return ERR_RTP_UDPV4TRANS_CANNOTFINSIH_OVERLAPPPED;
	}

	if (receivemode != RTPTransmitter::AcceptAll && 
		!ShouldAcceptData(ntohl(src_addr->sin_addr.s_addr),ntohs(src_addr->sin_port)))
	{
		return 0;
	}			
	
	RTPIPv4Address *addr = RTPNew(GetMemoryManager(),RTPMEM_TYPE_CLASS_RTPADDRESS) RTPIPv4Address(ntohl(src_addr->sin_addr.s_addr),ntohs(src_addr->sin_port));
	if (addr == 0)
	{
		return ERR_RTP_OUTOFMEM;
	}

	::uint8_t *datacopy = RTPNew(GetMemoryManager(),(rtp)?RTPMEM_TYPE_BUFFER_RECEIVEDRTPPACKET:RTPMEM_TYPE_BUFFER_RECEIVEDRTCPPACKET) ::uint8_t[recvlen];
	if (datacopy == 0)
	{
		RTPDelete(addr,GetMemoryManager());
		return ERR_RTP_OUTOFMEM;
	}


	RTPRawPacket *pack = RTPNew(GetMemoryManager(),RTPMEM_TYPE_CLASS_RTPRAWPACKET) RTPRawPacket(datacopy,recvlen,addr,curtime,rtp,GetMemoryManager());
	if (pack == 0)
	{
		RTPDelete(addr,GetMemoryManager());
		RTPDeleteByteArray(datacopy,GetMemoryManager());
		return ERR_RTP_OUTOFMEM;
	}

	rawpacketlist.push_back(pack);

	return IssueAsyncRead(rtp);
}

DWORD 
AsyncIocpRTPUDPv4Transmitter::RtpIocpKey() const 
{ 
	return _rtpIocpKey; 
}

DWORD 
AsyncIocpRTPUDPv4Transmitter::RtcpIocpKey() const 
{ 
	return _rtcpIocpKey; 
}


AsyncIocpRTPUDPv4Transmitter::~AsyncIocpRTPUDPv4Transmitter(void)
{
}

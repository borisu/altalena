#include "StdAfx.h"
#include "CcuLogger.h"
#include "AsyncIocpRTPUDPv4Transmitter.h"

typedef
std::list<::uint32_t> IpAddrList; 

#define RTPUDPV4TRANS_MAXPACKSIZE							65535
#define RTPUDPV4TRANS_IFREQBUFSIZE							8192

#define SIO_UDP_CONNRESET           _WSAIOW(IOC_VENDOR,12) 

AsyncIocpRTPUDPv4Transmitter::AsyncIocpRTPUDPv4Transmitter(
	IN RTPMemoryManager *mgr, 
	IN HANDLE iocpHandle)
:RTPUDPv4Transmitter(mgr),
_iocpHandle(iocpHandle)
{
}


int 
RTPUDPv4Transmitter::Create(IN size_t maximumpacketsize,
							IN const RTPTransmissionParams *transparams)
{

#pragma TODO ("Unlike other transmitters this one is not multi threaded and built for Win32 API.")

	const RTPUDPv4TransmissionParams *params = NULL;
	const RTPUDPv4TransmissionParams defaultparams;

	struct sockaddr_in addr;
	::memset(&addr,0,sizeof(addr));

	int size	= 0;
	int status  = 0;

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
	rtpsock = ::WSASocket(PF_INET, SOCK_DGRAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	if (rtpsock == INVALID_SOCKET)
	{
		LogSysError("::WSASocket");
		return ERR_RTP_UDPV4TRANS_CANTCREATESOCKET;
	}

	rtcpsock = ::WSASocket(PF_INET, SOCK_DGRAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
	if (rtpsock == INVALID_SOCKET)
	{
		LogSysError("::WSASocket");
		return ERR_RTP_UDPV4TRANS_CANTCREATESOCKET;
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

#pragma warning (suppress: 4244)
			RTPIPv4Address a(*it);
			LogDebug(a.GetAddressString());;

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

AsyncIocpRTPUDPv4Transmitter::~AsyncIocpRTPUDPv4Transmitter(void)
{
}

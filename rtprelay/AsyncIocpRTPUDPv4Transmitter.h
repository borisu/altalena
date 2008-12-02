#pragma once
#include "rtpudpv4transmitter.h"

class AsyncIocpRTPUDPv4Transmitter :
	public RTPUDPv4Transmitter
{
public:

	AsyncIocpRTPUDPv4Transmitter(RTPMemoryManager *mgr, HANDLE iocpHandle);

	~AsyncIocpRTPUDPv4Transmitter(void);

private:

	HANDLE _iocpHandle;
};

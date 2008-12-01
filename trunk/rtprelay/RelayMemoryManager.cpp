#include "StdAfx.h"
#include "RelayMemoryManager.h"

#pragma TODO("Implement efficient memory allocations for :")
#pragma TODO("RTPMEM_TYPE_BUFFER_RECEIVEDRTPPACKET")
#pragma TODO("RTPMEM_TYPE_CLASS_RTPADDRESS")
#pragma TODO("RTPMEM_TYPE_CLASS_RTPPACKET")
#pragma TODO("RTPMEM_TYPE_CLASS_RTPRAWPACKET")
#pragma TODO("RTPMEM_TYPE_RTP_MSG_RECEIVE")

using namespace std;

typedef 
map<int,int> MemoryObjectMap; 

MemoryObjectMap _memoryObjectCounters;

RelayMemoryManager::RelayMemoryManager(void)
{
}

RelayMemoryManager::~RelayMemoryManager(void)
{
}

void*
RelayMemoryManager::AllocateBuffer(size_t numbytes, int memtype)
{
	return operator new(numbytes);
}


void
RelayMemoryManager::FreeBuffer(void *buffer)
{
	operator delete (buffer);
}


#pragma once
#include "CcuLogger.h"
#include "rtpmemorymanager.h"

class RelayMemoryManager:
	public RTPMemoryManager
{
public:

	RelayMemoryManager(void);

	virtual ~RelayMemoryManager(void);

	virtual void *AllocateBuffer(size_t numbytes, int memtype);

	virtual void FreeBuffer(void *buffer);

};

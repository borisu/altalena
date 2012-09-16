#include "StdAfx.h"
#include "ComUtils.h"

ComUtils::ComUtils(void)
{
}

ComUtils::~ComUtils(void)
{
}


HRESULT ComUtils::CopyString(const OLECHAR * source, BSTR* dest)
{
	HRESULT hr = E_INVALIDARG;
	
	INT res = SysReAllocString(dest, source);
	hr = res == TRUE ? S_OK : E_OUTOFMEMORY;
	
	return hr;
}
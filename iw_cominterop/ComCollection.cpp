// ComCollection.cpp : Implementation of CComCollection

#include "stdafx.h"
#include "ComCollection.h"

using namespace MapColl;
// CComCollection

// ATL's collection implementation only allows numeric indexing (due to container-independence)
// so we have to provide our own implementation to allow string indexing into the map
HRESULT CComCollection::get_Item(VARIANT Index, VARIANT* pVal)
{
	if (pVal == NULL)
		return E_POINTER;

	HRESULT hr = S_OK;
	CComVariant var;

	if (Index.vt != VT_BSTR)
	{
		// If the index isn't a string, but can be converted to a long value, we use ATL's implementation
		hr = var.ChangeType(VT_I4, &Index);
		if (SUCCEEDED(hr))
			return CollectionType::get_Item(var.lVal, pVal);
	}

	// Otherwise, we assume index is a key into the map
	hr = var.ChangeType(VT_BSTR, &Index);

	// If we can't convert to a string, just return
	if (FAILED(hr))
		return hr;

	ContainerType::iterator it = m_coll.find(CComBSTR(var.bstrVal));

	// If item not found, just return
	if (it == m_coll.end())
		return E_FAIL;

	// If item was found, copy the variant to the out param
	return (CComVariant(it->second).Detach(pVal));
}

HRESULT CComCollection::Add(VARIANT Key, VARIANT Value)
{
	// Get a BSTR from the VARIANT
	CComBSTR str;
	HRESULT hr = VCUE::GenericCopy<BSTR, VARIANT>::copy(&str, &Key);

	// If we can't convert to a string, just return
	if (FAILED(hr))
		return hr;

	// Check whether item already exists
	if (m_coll.find(str) != m_coll.end())
		return E_FAIL;	// item with this key already exists

	// Add the item to the map
	m_coll[str] = Value;
	return S_OK;
}

HRESULT CComCollection::Remove(VARIANT Index)
{
	HRESULT hr = S_OK;
	CComVariant var;

	if (Index.vt != VT_BSTR)
	{
		// If the index isn't a string, but can be converted to a long value, we use the long implementation of Remove
		hr = var.ChangeType(VT_I4, &Index);
		if (SUCCEEDED(hr))
			return Remove(var.lVal);
	}

	// Otherwise, we assume index is a key into the map
	hr = var.ChangeType(VT_BSTR, &Index);

	// If we can't convert to a string, just return
	if (FAILED(hr))
		return hr;

	ContainerType::iterator it = m_coll.find(CComBSTR(var.bstrVal));

	// If item not found, just return
	if (it == m_coll.end())
		return E_FAIL;

	// If item was found, remove it from the map
	m_coll.erase(it);

	// (Could just use erase on the key, but it's not clear what erase does if the item isn't present.
	//  Our method is safer and allows us to inform the client of failure.)
	return S_OK;
}

HRESULT CComCollection::Remove(size_t Index)
{
	// NOTE: This code can be used with most STL containers

	// Check that the 1-based index is in bounds
	if (Index <= 0 || Index > m_coll.size())
		return E_INVALIDARG;

	// Get the iterator and erase the item
	ContainerType::iterator it = m_coll.begin();
	std::advance(it, Index - 1);
	m_coll.erase(it);

	return S_OK;
}
HRESULT CComCollection::Clear()
{
	// NOTE: This code can be used with most STL containers
	m_coll.clear();
	return S_OK;
}
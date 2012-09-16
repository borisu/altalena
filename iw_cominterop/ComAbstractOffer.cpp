// ComAbstractOffer.cpp : Implementation of CComAbstractOffer

#include "stdafx.h"
#include "ComAbstractOffer.h"
#include "ComUtils.h"


// CComAbstractOffer

#define GET_STR_PROPERTY(x) { _bstr_t bstrString = (x).c_str(); \
	*value = bstrString.copy(); };

#define PUT_STR_PROPERTY(x){ _bstr_t bstrString = value; \
	(x) = (char *)bstrString;};


/* [propget] */ HRESULT STDMETHODCALLTYPE CComAbstractOffer::get_Type( 
	/* [retval][out] */ BSTR *value)
{
	GET_STR_PROPERTY(_offer.type);
	return S_OK;
}

/* [propput] */ HRESULT STDMETHODCALLTYPE CComAbstractOffer::put_Type( 
	/* [in] */ BSTR *value)
{
	PUT_STR_PROPERTY(_offer.type);
	return S_OK;
}

/* [propget] */ HRESULT STDMETHODCALLTYPE CComAbstractOffer::get_Body( 
	/* [retval][out] */ BSTR *value)
{
	
	GET_STR_PROPERTY(_offer.body);
	return S_OK;

}

/* [propput] */ HRESULT STDMETHODCALLTYPE CComAbstractOffer::put_Body( 
	/* [in] */ BSTR *value)
{
	PUT_STR_PROPERTY(_offer.body);
	return S_OK;
}

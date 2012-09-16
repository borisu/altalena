// ComCredentials.cpp : Implementation of CComCredentials

#include "stdafx.h"
#include "ComCredentials.h"
#include "ComUtils.h"


// CComCredentials

/* [propget] */ HRESULT STDMETHODCALLTYPE CComCredentials::get_User( 
	/* [retval][out] */ BSTR *value)
{
	
	GET_STR_PROPERTY(_credentials.username);
	return S_OK;
}

/* [propput] */ HRESULT STDMETHODCALLTYPE CComCredentials::put_User( 
	/* [in] */ BSTR *value)
{
	PUT_STR_PROPERTY(_credentials.username);
	return S_OK;

}

/* [propget] */ HRESULT STDMETHODCALLTYPE CComCredentials::get_Password( 
	/* [retval][out] */ BSTR *value)
{
	GET_STR_PROPERTY(_credentials.password);
	return S_OK;

}

/* [propput] */ HRESULT STDMETHODCALLTYPE CComCredentials::put_Password( 
	/* [in] */ BSTR *value) 
{
	PUT_STR_PROPERTY(_credentials.password);
	return S_OK;

}

/* [propget] */ HRESULT STDMETHODCALLTYPE CComCredentials::get_Realm( 
	/* [retval][out] */ BSTR *value)
{
	GET_STR_PROPERTY(_credentials.realm);
	return S_OK;
}

/* [propput] */ HRESULT STDMETHODCALLTYPE CComCredentials::put_Realm( 
	/* [in] */ BSTR *value)
{
	PUT_STR_PROPERTY(_credentials.realm);
	return S_OK;
}
// ComSIPCall.cpp : Implementation of CComSIPCall

#include "stdafx.h"
#include "ComSIPCall.h"


// CComSIPCall


HRESULT STDMETHODCALLTYPE CComSIPCall::WaitForDtmf( 
									  /* [out] */ BSTR *signal,
									  /* [in] */ long timeout,
									  /* [retval][out] */ IwErrorCode *errCode)
{

	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::RejectCall( 
									 /* [retval][out] */ IwErrorCode *errCode)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::HangupCall( 
									 /* [retval][out] */ IwErrorCode *errCode)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::BlindXfer( 
									/* [ref][in] */ const BSTR *destination_uri,
									/* [retval][out] */ IwErrorCode *errCode)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::WaitTillHangup( void)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::Dnis( 
							   /* [ref][in] */ const BSTR *destination_uri)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::Ani( 
							  /* [ref][in] */ const BSTR *destination_uri)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::LocalOffer( 
									 /* [retval][out] */ IComAbstractOffer **localOffer)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::RemoteOffer( 
									  /* [retval][out] */ IComAbstractOffer **localOffer)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::MakeCall( 
								   /* [ref][in] */ const BSTR *destinationUri,
								   /* [in] */ IComAbstractOffer *localOffer,
								   /* [in] */ IComCredentials *credentials,
								   /* [in] */ IComMap *keyValueMap,
								   /* [in] */ long ringTimeout,
								   /* [retval][out] */ IwErrorCode *errCode)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::ReOffer( 
								  /* [in] */ IComAbstractOffer *localOffer,
								  /* [in] */ IComMap *keyValueMap,
								  /* [in] */ long ringTimeout,
								  /* [retval][out] */ IwErrorCode *errCode)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::Answer( 
								 /* [in] */ IComAbstractOffer *localOffer,
								 /* [in] */ IComMap *keyValueMap,
								 /* [in] */ long ringTimeout,
								 /* [retval][out] */ IwErrorCode *errCode)
{
	return S_OK;

}

HRESULT STDMETHODCALLTYPE CComSIPCall::DtmfBuffer( 
									 /* [retval][out] */ IwErrorCode *dtmfBuffer)
{
	return S_OK;

}
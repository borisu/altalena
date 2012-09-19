// ComSIPCall.h : Declaration of the CComSIPCall

#pragma once
#include "resource.h"       // main symbols

#include "iw_cominterop.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CComSIPCall

class ATL_NO_VTABLE CComSIPCall :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComSIPCall, &CLSID_ComSIPCall>,
	public IDispatchImpl<IComSIPCall, &IID_IComSIPCall, &LIBID_iw_cominteropLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CComSIPCall()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMSIPCALL)


BEGIN_COM_MAP(CComSIPCall)
	COM_INTERFACE_ENTRY(IComSIPCall)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:


	 HRESULT STDMETHODCALLTYPE WaitForDtmf( 
		/* [out] */ BSTR *signal,
		/* [in] */ long timeout,
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE RejectCall( 
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE HangupCall( 
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE BlindXfer( 
		/* [ref][in] */ const BSTR *destination_uri,
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE WaitTillHangup( void);

	 HRESULT STDMETHODCALLTYPE Dnis( 
		/* [ref][in] */ const BSTR *destination_uri);

	 HRESULT STDMETHODCALLTYPE Ani( 
		/* [ref][in] */ const BSTR *destination_uri);

	 HRESULT STDMETHODCALLTYPE LocalOffer( 
		/* [retval][out] */ IComAbstractOffer **localOffer);

	 HRESULT STDMETHODCALLTYPE RemoteOffer( 
		/* [retval][out] */ IComAbstractOffer **localOffer);

	 HRESULT STDMETHODCALLTYPE MakeCall( 
		/* [ref][in] */ const BSTR *destinationUri,
		/* [in] */ IComAbstractOffer *localOffer,
		/* [in] */ IComCredentials *credentials,
		/* [in] */ IComMap *keyValueMap,
		/* [in] */ long ringTimeout,
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE ReOffer( 
		/* [in] */ IComAbstractOffer *localOffer,
		/* [in] */ IComMap *keyValueMap,
		/* [in] */ long ringTimeout,
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE Answer( 
		/* [in] */ IComAbstractOffer *localOffer,
		/* [in] */ IComMap *keyValueMap,
		/* [in] */ long ringTimeout,
		/* [retval][out] */ IwErrorCode *errCode);

	 HRESULT STDMETHODCALLTYPE DtmfBuffer( 
		/* [retval][out] */ IwErrorCode *dtmfBuffer);

	
	
};

OBJECT_ENTRY_AUTO(__uuidof(ComSIPCall), CComSIPCall)

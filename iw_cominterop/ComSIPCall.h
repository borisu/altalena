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


	STDMETHOD(CleanDtmfBuffer)();

	STDMETHOD(WaitForDtmf)(
		OUT BSTR * signal, 
		IN  long timeout,
		OUT enum ApiErrorCode *errCode);

// 	virtual ApiErrorCode RejectCall();
// 
// 	virtual void UponCallTerminated(
// 		IN IwMessagePtr ptr);
// 
// 	virtual void UponActiveObjectEvent(
// 		IN IwMessagePtr ptr);
// 
// 	virtual ApiErrorCode HangupCall();
// 
// 	virtual ApiErrorCode BlindXfer(
// 		IN const string &destination_uri);
// 
// 	virtual void WaitTillHangup();
// 
// 	virtual const string& Dnis();
// 
// 	virtual const string& Ani();
// 
// 	virtual AbstractOffer LocalOffer();
// 
// 	virtual AbstractOffer RemoteOffer();
// 
// 	virtual ApiErrorCode MakeCall(
// 		IN const string			&destinationUri, 
// 		IN const AbstractOffer  &localOffer,
// 		IN const Credentials	&credentials, 
// 		IN OUT  MapOfAny		&keyValueMap,
// 		IN csp::Time			ringTimeout);
// 
// 	virtual ApiErrorCode ReOffer(
// 		IN const AbstractOffer  &localOffer,
// 		IN OUT  MapOfAny		&keyValueMap,
// 		IN csp::Time			ringTimeout);
// 
// 	virtual ApiErrorCode Answer(
// 		IN const AbstractOffer  &localOffer,
// 		IN const MapOfAny &keyValueMap,
// 		IN csp::Time	  ringTimeout);
// 
// 	virtual void UponDtmfEvt(
// 		IN IwMessagePtr ptr);
// 
// 	virtual string DtmfBuffer();
// 
// 	virtual int StackCallHandle() const;

	
	
};

OBJECT_ENTRY_AUTO(__uuidof(ComSIPCall), CComSIPCall)

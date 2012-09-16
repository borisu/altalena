// ComCredentials.h : Declaration of the CComCredentials

#pragma once
#include "resource.h"       // main symbols

#include "iw_cominterop.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CComCredentials

class ATL_NO_VTABLE CComCredentials :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComCredentials, &CLSID_ComCredentials>,
	public IDispatchImpl<IComCredentials, &IID_IComCredentials, &LIBID_iw_cominteropLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CComCredentials()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMCREDENTIALS)


BEGIN_COM_MAP(CComCredentials)
	COM_INTERFACE_ENTRY(IComCredentials)
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

	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_User( 
		/* [retval][out] */ BSTR *value);

	virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_User( 
		/* [in] */ BSTR *value);

	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Password( 
		/* [retval][out] */ BSTR *value);

	virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Password( 
		/* [in] */ BSTR *value);

	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Realm( 
		/* [retval][out] */ BSTR *value);

	virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Realm( 
		/* [in] */ BSTR *value);

public:

	ivrworx::Credentials _credentials;
	

};

OBJECT_ENTRY_AUTO(__uuidof(ComCredentials), CComCredentials)

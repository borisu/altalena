// ComAbstractOffer.h : Declaration of the CComAbstractOffer

#pragma once
#include "resource.h"       // main symbols

#include "iw_cominterop.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif




// CComAbstractOffer

class ATL_NO_VTABLE CComAbstractOffer :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComAbstractOffer, &CLSID_ComAbstractOffer>,
	public IDispatchImpl<IComAbstractOffer, &IID_IComAbstractOffer, &LIBID_iw_cominteropLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CComAbstractOffer()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMABSTRACTOFFER)


BEGIN_COM_MAP(CComAbstractOffer)
	COM_INTERFACE_ENTRY(IComAbstractOffer)
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

	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
		/* [retval][out] */ BSTR *value);

	virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
		/* [in] */ BSTR *value);

	virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Body( 
		/* [retval][out] */ BSTR *value);

	virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Body( 
		/* [in] */ BSTR *value);

	

private:

	AbstractOffer _offer;

};

OBJECT_ENTRY_AUTO(__uuidof(ComAbstractOffer), CComAbstractOffer)

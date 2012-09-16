// ComCollection.h : Declaration of the CComCollection

#pragma once
#include "resource.h"       // main symbols

#include "iw_cominterop.h"
#include "Reuse\VCUE_Collection.h"
#include "Reuse\VCUE_Copy.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

namespace MapColl
{
	// We always need to provide the following information
	typedef std::map< CAdapt< CComBSTR >, CComVariant >				ContainerType;
	typedef VARIANT													ExposedType;
	typedef IEnumVARIANT											EnumeratorInterface;
	typedef IComMap													CollectionInterface;

	// Typically the copy policy can be calculated from the typedefs defined above:
	// typedef VCUE::GenericCopy<ExposedType, ContainerType::value_type>		CopyType;

	// However, we may want to use a different class, as in this case:
	typedef VCUE::MapCopy<ContainerType, ExposedType>				CopyType;
	// (The advantage of MapCopy is that we don't need to provide implementations 
	//  of GenericCopy for all the different pairs of key and value types)

	// Now we have all the information we need to fill in the template arguments on the implementation classes
	typedef CComEnumOnSTL< EnumeratorInterface, &__uuidof(EnumeratorInterface), ExposedType,
		CopyType, ContainerType > EnumeratorType;

	typedef VCUE::ICollectionOnSTLCopyImpl< CollectionInterface, ContainerType, ExposedType,
		CopyType, EnumeratorType > CollectionType;
};

// CComCollection

class ATL_NO_VTABLE CComCollection :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CComCollection, &CLSID_ComMap>,
	public IDispatchImpl<MapColl::CollectionType, &IID_IComMap, &LIBID_iw_cominteropLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CComCollection()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_COMCOLLECTION)


BEGIN_COM_MAP(CComCollection)
	COM_INTERFACE_ENTRY(IComMap)
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

	// ATL's collection implementation only allows numeric indexing (due to container-independence)
	// so we have to provide our own implementation to allow string indexing into the map
	STDMETHOD(get_Item)(VARIANT Index, VARIANT* pVal);

	STDMETHOD(Add)(VARIANT Key, VARIANT Value);

	STDMETHOD(Remove)(VARIANT Index);

	STDMETHOD(Remove)(size_t Index);

	STDMETHOD(Clear)();

};

OBJECT_ENTRY_AUTO(__uuidof(ComMap), CComCollection)



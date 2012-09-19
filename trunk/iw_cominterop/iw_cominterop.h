

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Wed Sep 19 10:27:22 2012
 */
/* Compiler settings for .\iw_cominterop.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __iw_cominterop_h__
#define __iw_cominterop_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IComMap_FWD_DEFINED__
#define __IComMap_FWD_DEFINED__
typedef interface IComMap IComMap;
#endif 	/* __IComMap_FWD_DEFINED__ */


#ifndef __IComIVRWORX_FWD_DEFINED__
#define __IComIVRWORX_FWD_DEFINED__
typedef interface IComIVRWORX IComIVRWORX;
#endif 	/* __IComIVRWORX_FWD_DEFINED__ */


#ifndef __IComCredentials_FWD_DEFINED__
#define __IComCredentials_FWD_DEFINED__
typedef interface IComCredentials IComCredentials;
#endif 	/* __IComCredentials_FWD_DEFINED__ */


#ifndef __IComAbstractOffer_FWD_DEFINED__
#define __IComAbstractOffer_FWD_DEFINED__
typedef interface IComAbstractOffer IComAbstractOffer;
#endif 	/* __IComAbstractOffer_FWD_DEFINED__ */


#ifndef __IComSIPCall_FWD_DEFINED__
#define __IComSIPCall_FWD_DEFINED__
typedef interface IComSIPCall IComSIPCall;
#endif 	/* __IComSIPCall_FWD_DEFINED__ */


#ifndef __ComSIPCall_FWD_DEFINED__
#define __ComSIPCall_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComSIPCall ComSIPCall;
#else
typedef struct ComSIPCall ComSIPCall;
#endif /* __cplusplus */

#endif 	/* __ComSIPCall_FWD_DEFINED__ */


#ifndef __ComAbstractOffer_FWD_DEFINED__
#define __ComAbstractOffer_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComAbstractOffer ComAbstractOffer;
#else
typedef struct ComAbstractOffer ComAbstractOffer;
#endif /* __cplusplus */

#endif 	/* __ComAbstractOffer_FWD_DEFINED__ */


#ifndef __ComCredentials_FWD_DEFINED__
#define __ComCredentials_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComCredentials ComCredentials;
#else
typedef struct ComCredentials ComCredentials;
#endif /* __cplusplus */

#endif 	/* __ComCredentials_FWD_DEFINED__ */


#ifndef __ComMap_FWD_DEFINED__
#define __ComMap_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComMap ComMap;
#else
typedef struct ComMap ComMap;
#endif /* __cplusplus */

#endif 	/* __ComMap_FWD_DEFINED__ */


#ifndef __ComIVRWORX_FWD_DEFINED__
#define __ComIVRWORX_FWD_DEFINED__

#ifdef __cplusplus
typedef class ComIVRWORX ComIVRWORX;
#else
typedef struct ComIVRWORX ComIVRWORX;
#endif /* __cplusplus */

#endif 	/* __ComIVRWORX_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IComMap_INTERFACE_DEFINED__
#define __IComMap_INTERFACE_DEFINED__

/* interface IComMap */
/* [unique][helpstring][nonextensible][dual][object][uuid] */ 


EXTERN_C const IID IID_IComMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E9A35B70-8738-4F38-A685-ECA5B2CBCA59")
    IComMap : public IDispatch
    {
    public:
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get__NewEnum( 
            /* [retval][out] */ IUnknown **ppUnk) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Item( 
            /* [in] */ VARIANT Index,
            /* [retval][out] */ VARIANT *pVal) = 0;
        
        virtual /* [propget][id] */ HRESULT STDMETHODCALLTYPE get_Count( 
            /* [retval][out] */ long *pVal) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Add( 
            /* [in] */ VARIANT Key,
            /* [in] */ VARIANT Item) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Remove( 
            /* [in] */ VARIANT Index) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComMap * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComMap * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComMap * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComMap * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComMap * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComMap * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IComMap * This,
            /* [retval][out] */ IUnknown **ppUnk);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IComMap * This,
            /* [in] */ VARIANT Index,
            /* [retval][out] */ VARIANT *pVal);
        
        /* [propget][id] */ HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IComMap * This,
            /* [retval][out] */ long *pVal);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Add )( 
            IComMap * This,
            /* [in] */ VARIANT Key,
            /* [in] */ VARIANT Item);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IComMap * This,
            /* [in] */ VARIANT Index);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Clear )( 
            IComMap * This);
        
        END_INTERFACE
    } IComMapVtbl;

    interface IComMap
    {
        CONST_VTBL struct IComMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComMap_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComMap_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComMap_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComMap_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComMap_get__NewEnum(This,ppUnk)	\
    (This)->lpVtbl -> get__NewEnum(This,ppUnk)

#define IComMap_get_Item(This,Index,pVal)	\
    (This)->lpVtbl -> get_Item(This,Index,pVal)

#define IComMap_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IComMap_Add(This,Key,Item)	\
    (This)->lpVtbl -> Add(This,Key,Item)

#define IComMap_Remove(This,Index)	\
    (This)->lpVtbl -> Remove(This,Index)

#define IComMap_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget][id] */ HRESULT STDMETHODCALLTYPE IComMap_get__NewEnum_Proxy( 
    IComMap * This,
    /* [retval][out] */ IUnknown **ppUnk);


void __RPC_STUB IComMap_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IComMap_get_Item_Proxy( 
    IComMap * This,
    /* [in] */ VARIANT Index,
    /* [retval][out] */ VARIANT *pVal);


void __RPC_STUB IComMap_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget][id] */ HRESULT STDMETHODCALLTYPE IComMap_get_Count_Proxy( 
    IComMap * This,
    /* [retval][out] */ long *pVal);


void __RPC_STUB IComMap_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IComMap_Add_Proxy( 
    IComMap * This,
    /* [in] */ VARIANT Key,
    /* [in] */ VARIANT Item);


void __RPC_STUB IComMap_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IComMap_Remove_Proxy( 
    IComMap * This,
    /* [in] */ VARIANT Index);


void __RPC_STUB IComMap_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [id] */ HRESULT STDMETHODCALLTYPE IComMap_Clear_Proxy( 
    IComMap * This);


void __RPC_STUB IComMap_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComMap_INTERFACE_DEFINED__ */


#ifndef __IComIVRWORX_INTERFACE_DEFINED__
#define __IComIVRWORX_INTERFACE_DEFINED__

/* interface IComIVRWORX */
/* [unique][helpstring][nonextensible][dual][object][uuid] */ 


EXTERN_C const IID IID_IComIVRWORX;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BABCF77D-476B-4E79-8FC0-8EA14FB6AF8D")
    IComIVRWORX : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ BSTR conf_file) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComIVRWORXVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComIVRWORX * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComIVRWORX * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComIVRWORX * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComIVRWORX * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComIVRWORX * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComIVRWORX * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComIVRWORX * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IComIVRWORX * This,
            /* [in] */ BSTR conf_file);
        
        END_INTERFACE
    } IComIVRWORXVtbl;

    interface IComIVRWORX
    {
        CONST_VTBL struct IComIVRWORXVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComIVRWORX_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComIVRWORX_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComIVRWORX_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComIVRWORX_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComIVRWORX_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComIVRWORX_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComIVRWORX_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComIVRWORX_Init(This,conf_file)	\
    (This)->lpVtbl -> Init(This,conf_file)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IComIVRWORX_Init_Proxy( 
    IComIVRWORX * This,
    /* [in] */ BSTR conf_file);


void __RPC_STUB IComIVRWORX_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComIVRWORX_INTERFACE_DEFINED__ */


#ifndef __IComCredentials_INTERFACE_DEFINED__
#define __IComCredentials_INTERFACE_DEFINED__

/* interface IComCredentials */
/* [unique][helpstring][nonextensible][dual][object][uuid] */ 


EXTERN_C const IID IID_IComCredentials;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90C8F27F-C8AF-4CF3-BCF4-9F838AC42A17")
    IComCredentials : public IDispatch
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_User( 
            /* [retval][out] */ BSTR *value) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_User( 
            /* [in] */ BSTR *value) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Password( 
            /* [retval][out] */ BSTR *value) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Password( 
            /* [in] */ BSTR *value) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Realm( 
            /* [retval][out] */ BSTR *value) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Realm( 
            /* [in] */ BSTR *value) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComCredentialsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComCredentials * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComCredentials * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComCredentials * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComCredentials * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComCredentials * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComCredentials * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComCredentials * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_User )( 
            IComCredentials * This,
            /* [retval][out] */ BSTR *value);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_User )( 
            IComCredentials * This,
            /* [in] */ BSTR *value);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Password )( 
            IComCredentials * This,
            /* [retval][out] */ BSTR *value);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Password )( 
            IComCredentials * This,
            /* [in] */ BSTR *value);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Realm )( 
            IComCredentials * This,
            /* [retval][out] */ BSTR *value);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Realm )( 
            IComCredentials * This,
            /* [in] */ BSTR *value);
        
        END_INTERFACE
    } IComCredentialsVtbl;

    interface IComCredentials
    {
        CONST_VTBL struct IComCredentialsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComCredentials_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComCredentials_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComCredentials_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComCredentials_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComCredentials_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComCredentials_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComCredentials_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComCredentials_get_User(This,value)	\
    (This)->lpVtbl -> get_User(This,value)

#define IComCredentials_put_User(This,value)	\
    (This)->lpVtbl -> put_User(This,value)

#define IComCredentials_get_Password(This,value)	\
    (This)->lpVtbl -> get_Password(This,value)

#define IComCredentials_put_Password(This,value)	\
    (This)->lpVtbl -> put_Password(This,value)

#define IComCredentials_get_Realm(This,value)	\
    (This)->lpVtbl -> get_Realm(This,value)

#define IComCredentials_put_Realm(This,value)	\
    (This)->lpVtbl -> put_Realm(This,value)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IComCredentials_get_User_Proxy( 
    IComCredentials * This,
    /* [retval][out] */ BSTR *value);


void __RPC_STUB IComCredentials_get_User_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IComCredentials_put_User_Proxy( 
    IComCredentials * This,
    /* [in] */ BSTR *value);


void __RPC_STUB IComCredentials_put_User_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IComCredentials_get_Password_Proxy( 
    IComCredentials * This,
    /* [retval][out] */ BSTR *value);


void __RPC_STUB IComCredentials_get_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IComCredentials_put_Password_Proxy( 
    IComCredentials * This,
    /* [in] */ BSTR *value);


void __RPC_STUB IComCredentials_put_Password_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IComCredentials_get_Realm_Proxy( 
    IComCredentials * This,
    /* [retval][out] */ BSTR *value);


void __RPC_STUB IComCredentials_get_Realm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IComCredentials_put_Realm_Proxy( 
    IComCredentials * This,
    /* [in] */ BSTR *value);


void __RPC_STUB IComCredentials_put_Realm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComCredentials_INTERFACE_DEFINED__ */


#ifndef __IComAbstractOffer_INTERFACE_DEFINED__
#define __IComAbstractOffer_INTERFACE_DEFINED__

/* interface IComAbstractOffer */
/* [unique][helpstring][nonextensible][dual][object][uuid] */ 


EXTERN_C const IID IID_IComAbstractOffer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("19E2D99B-3FE2-40BB-BF2E-08E89A3038C9")
    IComAbstractOffer : public IDispatch
    {
    public:
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Type( 
            /* [retval][out] */ BSTR *value) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Type( 
            /* [in] */ BSTR *value) = 0;
        
        virtual /* [propget] */ HRESULT STDMETHODCALLTYPE get_Body( 
            /* [retval][out] */ BSTR *value) = 0;
        
        virtual /* [propput] */ HRESULT STDMETHODCALLTYPE put_Body( 
            /* [in] */ BSTR *value) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComAbstractOfferVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComAbstractOffer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComAbstractOffer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComAbstractOffer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComAbstractOffer * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComAbstractOffer * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComAbstractOffer * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComAbstractOffer * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IComAbstractOffer * This,
            /* [retval][out] */ BSTR *value);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IComAbstractOffer * This,
            /* [in] */ BSTR *value);
        
        /* [propget] */ HRESULT ( STDMETHODCALLTYPE *get_Body )( 
            IComAbstractOffer * This,
            /* [retval][out] */ BSTR *value);
        
        /* [propput] */ HRESULT ( STDMETHODCALLTYPE *put_Body )( 
            IComAbstractOffer * This,
            /* [in] */ BSTR *value);
        
        END_INTERFACE
    } IComAbstractOfferVtbl;

    interface IComAbstractOffer
    {
        CONST_VTBL struct IComAbstractOfferVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComAbstractOffer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComAbstractOffer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComAbstractOffer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComAbstractOffer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComAbstractOffer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComAbstractOffer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComAbstractOffer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComAbstractOffer_get_Type(This,value)	\
    (This)->lpVtbl -> get_Type(This,value)

#define IComAbstractOffer_put_Type(This,value)	\
    (This)->lpVtbl -> put_Type(This,value)

#define IComAbstractOffer_get_Body(This,value)	\
    (This)->lpVtbl -> get_Body(This,value)

#define IComAbstractOffer_put_Body(This,value)	\
    (This)->lpVtbl -> put_Body(This,value)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [propget] */ HRESULT STDMETHODCALLTYPE IComAbstractOffer_get_Type_Proxy( 
    IComAbstractOffer * This,
    /* [retval][out] */ BSTR *value);


void __RPC_STUB IComAbstractOffer_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IComAbstractOffer_put_Type_Proxy( 
    IComAbstractOffer * This,
    /* [in] */ BSTR *value);


void __RPC_STUB IComAbstractOffer_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propget] */ HRESULT STDMETHODCALLTYPE IComAbstractOffer_get_Body_Proxy( 
    IComAbstractOffer * This,
    /* [retval][out] */ BSTR *value);


void __RPC_STUB IComAbstractOffer_get_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [propput] */ HRESULT STDMETHODCALLTYPE IComAbstractOffer_put_Body_Proxy( 
    IComAbstractOffer * This,
    /* [in] */ BSTR *value);


void __RPC_STUB IComAbstractOffer_put_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComAbstractOffer_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_iw_cominterop_0265 */
/* [local] */ 

typedef 
enum IwErrorCode
    {	IW_SUCCESS	= 0,
	IW_FAILURE	= IW_SUCCESS + 1,
	IW_SOCKET_INIT_FAILURE	= IW_FAILURE + 1,
	IW_TIMER_INIT_FAILURE	= IW_SOCKET_INIT_FAILURE + 1,
	IW_SERVER_FAILURE	= IW_TIMER_INIT_FAILURE + 1,
	IW_TIMEOUT	= IW_SERVER_FAILURE + 1,
	IW_WRONG_PARAMETER	= IW_TIMEOUT + 1,
	IW_WRONG_STATE	= IW_WRONG_PARAMETER + 1,
	IW_HANGUP	= IW_WRONG_STATE + 1,
	IW_UNKNOWN_DESTINATION	= IW_HANGUP + 1,
	IW_FEATURE_DISABLED	= IW_UNKNOWN_DESTINATION + 1,
	IW_UNKNOWN_RESPONSE	= IW_FEATURE_DISABLED + 1,
	IW_PENDING_OPERATION	= IW_UNKNOWN_RESPONSE + 1
    } 	IwErrorCode;



extern RPC_IF_HANDLE __MIDL_itf_iw_cominterop_0265_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_iw_cominterop_0265_v0_0_s_ifspec;

#ifndef __IComSIPCall_INTERFACE_DEFINED__
#define __IComSIPCall_INTERFACE_DEFINED__

/* interface IComSIPCall */
/* [unique][helpstring][nonextensible][dual][object][uuid] */ 


EXTERN_C const IID IID_IComSIPCall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("533A9932-E0F0-4B30-86CE-5B2BA3653E95")
    IComSIPCall : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WaitForDtmf( 
            /* [out] */ BSTR *signal,
            /* [in] */ long timeout,
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RejectCall( 
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE HangupCall( 
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BlindXfer( 
            /* [ref][in] */ const BSTR *destination_uri,
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WaitTillHangup( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Dnis( 
            /* [ref][in] */ const BSTR *destination_uri) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Ani( 
            /* [ref][in] */ const BSTR *destination_uri) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LocalOffer( 
            /* [retval][out] */ IComAbstractOffer **localOffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoteOffer( 
            /* [retval][out] */ IComAbstractOffer **localOffer) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MakeCall( 
            /* [ref][in] */ const BSTR *destinationUri,
            /* [in] */ IComAbstractOffer *localOffer,
            /* [in] */ IComCredentials *credentials,
            /* [in] */ IComMap *keyValueMap,
            /* [in] */ long ringTimeout,
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReOffer( 
            /* [in] */ IComAbstractOffer *localOffer,
            /* [in] */ IComMap *keyValueMap,
            /* [in] */ long ringTimeout,
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Answer( 
            /* [in] */ IComAbstractOffer *localOffer,
            /* [in] */ IComMap *keyValueMap,
            /* [in] */ long ringTimeout,
            /* [retval][out] */ IwErrorCode *errCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DtmfBuffer( 
            /* [retval][out] */ IwErrorCode *dtmfBuffer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IComSIPCallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IComSIPCall * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IComSIPCall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IComSIPCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IComSIPCall * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IComSIPCall * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IComSIPCall * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IComSIPCall * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *WaitForDtmf )( 
            IComSIPCall * This,
            /* [out] */ BSTR *signal,
            /* [in] */ long timeout,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *RejectCall )( 
            IComSIPCall * This,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *HangupCall )( 
            IComSIPCall * This,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *BlindXfer )( 
            IComSIPCall * This,
            /* [ref][in] */ const BSTR *destination_uri,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *WaitTillHangup )( 
            IComSIPCall * This);
        
        HRESULT ( STDMETHODCALLTYPE *Dnis )( 
            IComSIPCall * This,
            /* [ref][in] */ const BSTR *destination_uri);
        
        HRESULT ( STDMETHODCALLTYPE *Ani )( 
            IComSIPCall * This,
            /* [ref][in] */ const BSTR *destination_uri);
        
        HRESULT ( STDMETHODCALLTYPE *LocalOffer )( 
            IComSIPCall * This,
            /* [retval][out] */ IComAbstractOffer **localOffer);
        
        HRESULT ( STDMETHODCALLTYPE *RemoteOffer )( 
            IComSIPCall * This,
            /* [retval][out] */ IComAbstractOffer **localOffer);
        
        HRESULT ( STDMETHODCALLTYPE *MakeCall )( 
            IComSIPCall * This,
            /* [ref][in] */ const BSTR *destinationUri,
            /* [in] */ IComAbstractOffer *localOffer,
            /* [in] */ IComCredentials *credentials,
            /* [in] */ IComMap *keyValueMap,
            /* [in] */ long ringTimeout,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *ReOffer )( 
            IComSIPCall * This,
            /* [in] */ IComAbstractOffer *localOffer,
            /* [in] */ IComMap *keyValueMap,
            /* [in] */ long ringTimeout,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *Answer )( 
            IComSIPCall * This,
            /* [in] */ IComAbstractOffer *localOffer,
            /* [in] */ IComMap *keyValueMap,
            /* [in] */ long ringTimeout,
            /* [retval][out] */ IwErrorCode *errCode);
        
        HRESULT ( STDMETHODCALLTYPE *DtmfBuffer )( 
            IComSIPCall * This,
            /* [retval][out] */ IwErrorCode *dtmfBuffer);
        
        END_INTERFACE
    } IComSIPCallVtbl;

    interface IComSIPCall
    {
        CONST_VTBL struct IComSIPCallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IComSIPCall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IComSIPCall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IComSIPCall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IComSIPCall_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IComSIPCall_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IComSIPCall_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IComSIPCall_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IComSIPCall_WaitForDtmf(This,signal,timeout,errCode)	\
    (This)->lpVtbl -> WaitForDtmf(This,signal,timeout,errCode)

#define IComSIPCall_RejectCall(This,errCode)	\
    (This)->lpVtbl -> RejectCall(This,errCode)

#define IComSIPCall_HangupCall(This,errCode)	\
    (This)->lpVtbl -> HangupCall(This,errCode)

#define IComSIPCall_BlindXfer(This,destination_uri,errCode)	\
    (This)->lpVtbl -> BlindXfer(This,destination_uri,errCode)

#define IComSIPCall_WaitTillHangup(This)	\
    (This)->lpVtbl -> WaitTillHangup(This)

#define IComSIPCall_Dnis(This,destination_uri)	\
    (This)->lpVtbl -> Dnis(This,destination_uri)

#define IComSIPCall_Ani(This,destination_uri)	\
    (This)->lpVtbl -> Ani(This,destination_uri)

#define IComSIPCall_LocalOffer(This,localOffer)	\
    (This)->lpVtbl -> LocalOffer(This,localOffer)

#define IComSIPCall_RemoteOffer(This,localOffer)	\
    (This)->lpVtbl -> RemoteOffer(This,localOffer)

#define IComSIPCall_MakeCall(This,destinationUri,localOffer,credentials,keyValueMap,ringTimeout,errCode)	\
    (This)->lpVtbl -> MakeCall(This,destinationUri,localOffer,credentials,keyValueMap,ringTimeout,errCode)

#define IComSIPCall_ReOffer(This,localOffer,keyValueMap,ringTimeout,errCode)	\
    (This)->lpVtbl -> ReOffer(This,localOffer,keyValueMap,ringTimeout,errCode)

#define IComSIPCall_Answer(This,localOffer,keyValueMap,ringTimeout,errCode)	\
    (This)->lpVtbl -> Answer(This,localOffer,keyValueMap,ringTimeout,errCode)

#define IComSIPCall_DtmfBuffer(This,dtmfBuffer)	\
    (This)->lpVtbl -> DtmfBuffer(This,dtmfBuffer)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IComSIPCall_WaitForDtmf_Proxy( 
    IComSIPCall * This,
    /* [out] */ BSTR *signal,
    /* [in] */ long timeout,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_WaitForDtmf_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_RejectCall_Proxy( 
    IComSIPCall * This,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_RejectCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_HangupCall_Proxy( 
    IComSIPCall * This,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_HangupCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_BlindXfer_Proxy( 
    IComSIPCall * This,
    /* [ref][in] */ const BSTR *destination_uri,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_BlindXfer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_WaitTillHangup_Proxy( 
    IComSIPCall * This);


void __RPC_STUB IComSIPCall_WaitTillHangup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_Dnis_Proxy( 
    IComSIPCall * This,
    /* [ref][in] */ const BSTR *destination_uri);


void __RPC_STUB IComSIPCall_Dnis_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_Ani_Proxy( 
    IComSIPCall * This,
    /* [ref][in] */ const BSTR *destination_uri);


void __RPC_STUB IComSIPCall_Ani_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_LocalOffer_Proxy( 
    IComSIPCall * This,
    /* [retval][out] */ IComAbstractOffer **localOffer);


void __RPC_STUB IComSIPCall_LocalOffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_RemoteOffer_Proxy( 
    IComSIPCall * This,
    /* [retval][out] */ IComAbstractOffer **localOffer);


void __RPC_STUB IComSIPCall_RemoteOffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_MakeCall_Proxy( 
    IComSIPCall * This,
    /* [ref][in] */ const BSTR *destinationUri,
    /* [in] */ IComAbstractOffer *localOffer,
    /* [in] */ IComCredentials *credentials,
    /* [in] */ IComMap *keyValueMap,
    /* [in] */ long ringTimeout,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_MakeCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_ReOffer_Proxy( 
    IComSIPCall * This,
    /* [in] */ IComAbstractOffer *localOffer,
    /* [in] */ IComMap *keyValueMap,
    /* [in] */ long ringTimeout,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_ReOffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_Answer_Proxy( 
    IComSIPCall * This,
    /* [in] */ IComAbstractOffer *localOffer,
    /* [in] */ IComMap *keyValueMap,
    /* [in] */ long ringTimeout,
    /* [retval][out] */ IwErrorCode *errCode);


void __RPC_STUB IComSIPCall_Answer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IComSIPCall_DtmfBuffer_Proxy( 
    IComSIPCall * This,
    /* [retval][out] */ IwErrorCode *dtmfBuffer);


void __RPC_STUB IComSIPCall_DtmfBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IComSIPCall_INTERFACE_DEFINED__ */



#ifndef __iw_cominteropLib_LIBRARY_DEFINED__
#define __iw_cominteropLib_LIBRARY_DEFINED__

/* library iw_cominteropLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_iw_cominteropLib;

EXTERN_C const CLSID CLSID_ComSIPCall;

#ifdef __cplusplus

class DECLSPEC_UUID("1089F4B2-CE41-42E0-ACFC-F5DF2857279B")
ComSIPCall;
#endif

EXTERN_C const CLSID CLSID_ComAbstractOffer;

#ifdef __cplusplus

class DECLSPEC_UUID("BAA3251A-F75C-4D74-B5A9-5C60D5ABE631")
ComAbstractOffer;
#endif

EXTERN_C const CLSID CLSID_ComCredentials;

#ifdef __cplusplus

class DECLSPEC_UUID("016E3F43-B0D7-4AA5-B081-2E9BC829CBDE")
ComCredentials;
#endif

EXTERN_C const CLSID CLSID_ComMap;

#ifdef __cplusplus

class DECLSPEC_UUID("6D221A60-7DA4-4472-B1A8-84CE0ECD9BC3")
ComMap;
#endif

EXTERN_C const CLSID CLSID_ComIVRWORX;

#ifdef __cplusplus

class DECLSPEC_UUID("3D381FB1-C57E-4B74-9F86-3ED8B59549A7")
ComIVRWORX;
#endif
#endif /* __iw_cominteropLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif



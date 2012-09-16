

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0366 */
/* at Sun Sep 16 12:08:26 2012
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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IComMap,0xE9A35B70,0x8738,0x4F38,0xA6,0x85,0xEC,0xA5,0xB2,0xCB,0xCA,0x59);


MIDL_DEFINE_GUID(IID, IID_IComIVRWORX,0xBABCF77D,0x476B,0x4E79,0x8F,0xC0,0x8E,0xA1,0x4F,0xB6,0xAF,0x8D);


MIDL_DEFINE_GUID(IID, IID_IComCredentials,0x90C8F27F,0xC8AF,0x4CF3,0xBC,0xF4,0x9F,0x83,0x8A,0xC4,0x2A,0x17);


MIDL_DEFINE_GUID(IID, IID_IComAbstractOffer,0x19E2D99B,0x3FE2,0x40BB,0xBF,0x2E,0x08,0xE8,0x9A,0x30,0x38,0xC9);


MIDL_DEFINE_GUID(IID, IID_IComSIPCall,0x533A9932,0xE0F0,0x4B30,0x86,0xCE,0x5B,0x2B,0xA3,0x65,0x3E,0x95);


MIDL_DEFINE_GUID(IID, LIBID_iw_cominteropLib,0x64BFA26C,0xB548,0x490A,0xA2,0xEC,0x65,0xB0,0x94,0x87,0x08,0xFE);


MIDL_DEFINE_GUID(CLSID, CLSID_ComSIPCall,0x1089F4B2,0xCE41,0x42E0,0xAC,0xFC,0xF5,0xDF,0x28,0x57,0x27,0x9B);


MIDL_DEFINE_GUID(CLSID, CLSID_ComAbstractOffer,0xBAA3251A,0xF75C,0x4D74,0xB5,0xA9,0x5C,0x60,0xD5,0xAB,0xE6,0x31);


MIDL_DEFINE_GUID(CLSID, CLSID_ComCredentials,0x016E3F43,0xB0D7,0x4AA5,0xB0,0x81,0x2E,0x9B,0xC8,0x29,0xCB,0xDE);


MIDL_DEFINE_GUID(CLSID, CLSID_ComMap,0x6D221A60,0x7DA4,0x4472,0xB1,0xA8,0x84,0xCE,0x0E,0xCD,0x9B,0xC3);


MIDL_DEFINE_GUID(CLSID, CLSID_ComIVRWORX,0x3D381FB1,0xC57E,0x4B74,0x9F,0x86,0x3E,0xD8,0xB5,0x95,0x49,0xA7);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif




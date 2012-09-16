

/* this ALWAYS GENERATED file contains the proxy stub code */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */
#pragma warning( disable: 4211 )  /* redefine extent to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma optimize("", off ) 

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "iw_cominterop.h"

#define TYPE_FORMAT_STRING_SIZE   1143                              
#define PROC_FORMAT_STRING_SIZE   757                               
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   2            

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


static RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IComMap_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IComMap_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IComIVRWORX_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IComIVRWORX_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IComCredentials_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IComCredentials_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IComAbstractOffer_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IComAbstractOffer_ProxyInfo;


extern const MIDL_STUB_DESC Object_StubDesc;


extern const MIDL_SERVER_INFO IComSIPCall_ServerInfo;
extern const MIDL_STUBLESS_PROXY_INFO IComSIPCall_ProxyInfo;


extern const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ];

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, [wire_marshal] or [user_marshal] attribute.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure get__NewEnum */

			0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x7 ),	/* 7 */
/*  8 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 10 */	NdrFcShort( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x8 ),	/* 8 */
/* 14 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter ppUnk */

/* 16 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 18 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 20 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */

	/* Return value */

/* 22 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 24 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 26 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Item */

/* 28 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 30 */	NdrFcLong( 0x0 ),	/* 0 */
/* 34 */	NdrFcShort( 0x8 ),	/* 8 */
/* 36 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 38 */	NdrFcShort( 0x0 ),	/* 0 */
/* 40 */	NdrFcShort( 0x8 ),	/* 8 */
/* 42 */	0x7,		/* Oi2 Flags:  srv must size, clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter Index */

/* 44 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 46 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 48 */	NdrFcShort( 0x3e6 ),	/* Type Offset=998 */

	/* Parameter pVal */

/* 50 */	NdrFcShort( 0x4113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=16 */
/* 52 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 54 */	NdrFcShort( 0x3f8 ),	/* Type Offset=1016 */

	/* Return value */

/* 56 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 58 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 60 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Count */

/* 62 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 64 */	NdrFcLong( 0x0 ),	/* 0 */
/* 68 */	NdrFcShort( 0x9 ),	/* 9 */
/* 70 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 72 */	NdrFcShort( 0x0 ),	/* 0 */
/* 74 */	NdrFcShort( 0x24 ),	/* 36 */
/* 76 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter pVal */

/* 78 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 80 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 82 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Return value */

/* 84 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 86 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 88 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Add */

/* 90 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 92 */	NdrFcLong( 0x0 ),	/* 0 */
/* 96 */	NdrFcShort( 0xa ),	/* 10 */
/* 98 */	NdrFcShort( 0x28 ),	/* x86 Stack size/offset = 40 */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */
/* 102 */	NdrFcShort( 0x8 ),	/* 8 */
/* 104 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter Key */

/* 106 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 108 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 110 */	NdrFcShort( 0x3e6 ),	/* Type Offset=998 */

	/* Parameter Item */

/* 112 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 114 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 116 */	NdrFcShort( 0x3e6 ),	/* Type Offset=998 */

	/* Return value */

/* 118 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 120 */	NdrFcShort( 0x24 ),	/* x86 Stack size/offset = 36 */
/* 122 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Remove */

/* 124 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 126 */	NdrFcLong( 0x0 ),	/* 0 */
/* 130 */	NdrFcShort( 0xb ),	/* 11 */
/* 132 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 134 */	NdrFcShort( 0x0 ),	/* 0 */
/* 136 */	NdrFcShort( 0x8 ),	/* 8 */
/* 138 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter Index */

/* 140 */	NdrFcShort( 0x8b ),	/* Flags:  must size, must free, in, by val, */
/* 142 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 144 */	NdrFcShort( 0x3e6 ),	/* Type Offset=998 */

	/* Return value */

/* 146 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 148 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 150 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Clear */

/* 152 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 154 */	NdrFcLong( 0x0 ),	/* 0 */
/* 158 */	NdrFcShort( 0xc ),	/* 12 */
/* 160 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 162 */	NdrFcShort( 0x0 ),	/* 0 */
/* 164 */	NdrFcShort( 0x8 ),	/* 8 */
/* 166 */	0x4,		/* Oi2 Flags:  has return, */
			0x1,		/* 1 */

	/* Return value */

/* 168 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 170 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 172 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Type */


	/* Procedure get_User */

/* 174 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 176 */	NdrFcLong( 0x0 ),	/* 0 */
/* 180 */	NdrFcShort( 0x7 ),	/* 7 */
/* 182 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 184 */	NdrFcShort( 0x0 ),	/* 0 */
/* 186 */	NdrFcShort( 0x8 ),	/* 8 */
/* 188 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter value */


	/* Parameter value */

/* 190 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 192 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 194 */	NdrFcShort( 0x40e ),	/* Type Offset=1038 */

	/* Return value */


	/* Return value */

/* 196 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 198 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 200 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure put_Type */


	/* Procedure put_User */

/* 202 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 204 */	NdrFcLong( 0x0 ),	/* 0 */
/* 208 */	NdrFcShort( 0x8 ),	/* 8 */
/* 210 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 212 */	NdrFcShort( 0x0 ),	/* 0 */
/* 214 */	NdrFcShort( 0x8 ),	/* 8 */
/* 216 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter value */


	/* Parameter value */

/* 218 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 220 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 222 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */

	/* Return value */


	/* Return value */

/* 224 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 226 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 228 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Body */


	/* Procedure get_Password */

/* 230 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 232 */	NdrFcLong( 0x0 ),	/* 0 */
/* 236 */	NdrFcShort( 0x9 ),	/* 9 */
/* 238 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 240 */	NdrFcShort( 0x0 ),	/* 0 */
/* 242 */	NdrFcShort( 0x8 ),	/* 8 */
/* 244 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter value */


	/* Parameter value */

/* 246 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 248 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 250 */	NdrFcShort( 0x40e ),	/* Type Offset=1038 */

	/* Return value */


	/* Return value */

/* 252 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 254 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 256 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure put_Body */


	/* Procedure put_Password */

/* 258 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 260 */	NdrFcLong( 0x0 ),	/* 0 */
/* 264 */	NdrFcShort( 0xa ),	/* 10 */
/* 266 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 268 */	NdrFcShort( 0x0 ),	/* 0 */
/* 270 */	NdrFcShort( 0x8 ),	/* 8 */
/* 272 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter value */


	/* Parameter value */

/* 274 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 276 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 278 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */

	/* Return value */


	/* Return value */

/* 280 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 282 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 284 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure get_Realm */

/* 286 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 288 */	NdrFcLong( 0x0 ),	/* 0 */
/* 292 */	NdrFcShort( 0xb ),	/* 11 */
/* 294 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 296 */	NdrFcShort( 0x0 ),	/* 0 */
/* 298 */	NdrFcShort( 0x8 ),	/* 8 */
/* 300 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter value */

/* 302 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 304 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 306 */	NdrFcShort( 0x40e ),	/* Type Offset=1038 */

	/* Return value */

/* 308 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 310 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 312 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Dnis */


	/* Procedure put_Realm */

/* 314 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 316 */	NdrFcLong( 0x0 ),	/* 0 */
/* 320 */	NdrFcShort( 0xc ),	/* 12 */
/* 322 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 324 */	NdrFcShort( 0x0 ),	/* 0 */
/* 326 */	NdrFcShort( 0x8 ),	/* 8 */
/* 328 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter destination_uri */


	/* Parameter value */

/* 330 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 332 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 334 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */

	/* Return value */


	/* Return value */

/* 336 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 338 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 340 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure WaitForDtmf */

/* 342 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 344 */	NdrFcLong( 0x0 ),	/* 0 */
/* 348 */	NdrFcShort( 0x7 ),	/* 7 */
/* 350 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 352 */	NdrFcShort( 0x8 ),	/* 8 */
/* 354 */	NdrFcShort( 0x22 ),	/* 34 */
/* 356 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x4,		/* 4 */

	/* Parameter signal */

/* 358 */	NdrFcShort( 0x2113 ),	/* Flags:  must size, must free, out, simple ref, srv alloc size=8 */
/* 360 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 362 */	NdrFcShort( 0x40e ),	/* Type Offset=1038 */

	/* Parameter timeout */

/* 364 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 366 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 368 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errCode */

/* 370 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 372 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 374 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 376 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 378 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 380 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RejectCall */

/* 382 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 384 */	NdrFcLong( 0x0 ),	/* 0 */
/* 388 */	NdrFcShort( 0x8 ),	/* 8 */
/* 390 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 392 */	NdrFcShort( 0x0 ),	/* 0 */
/* 394 */	NdrFcShort( 0x22 ),	/* 34 */
/* 396 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter errCode */

/* 398 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 400 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 402 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 404 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 406 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 408 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure HangupCall */

/* 410 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 412 */	NdrFcLong( 0x0 ),	/* 0 */
/* 416 */	NdrFcShort( 0x9 ),	/* 9 */
/* 418 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 420 */	NdrFcShort( 0x0 ),	/* 0 */
/* 422 */	NdrFcShort( 0x22 ),	/* 34 */
/* 424 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter errCode */

/* 426 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 428 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 430 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 432 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 434 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 436 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure BlindXfer */

/* 438 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 440 */	NdrFcLong( 0x0 ),	/* 0 */
/* 444 */	NdrFcShort( 0xa ),	/* 10 */
/* 446 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 448 */	NdrFcShort( 0x0 ),	/* 0 */
/* 450 */	NdrFcShort( 0x22 ),	/* 34 */
/* 452 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x3,		/* 3 */

	/* Parameter destination_uri */

/* 454 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 456 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 458 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */

	/* Parameter errCode */

/* 460 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 462 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 464 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 466 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 468 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 470 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure WaitTillHangup */

/* 472 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 474 */	NdrFcLong( 0x0 ),	/* 0 */
/* 478 */	NdrFcShort( 0xb ),	/* 11 */
/* 480 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 482 */	NdrFcShort( 0x0 ),	/* 0 */
/* 484 */	NdrFcShort( 0x8 ),	/* 8 */
/* 486 */	0x4,		/* Oi2 Flags:  has return, */
			0x1,		/* 1 */

	/* Return value */

/* 488 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 490 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 492 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Ani */

/* 494 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 496 */	NdrFcLong( 0x0 ),	/* 0 */
/* 500 */	NdrFcShort( 0xd ),	/* 13 */
/* 502 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 504 */	NdrFcShort( 0x0 ),	/* 0 */
/* 506 */	NdrFcShort( 0x8 ),	/* 8 */
/* 508 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x2,		/* 2 */

	/* Parameter destination_uri */

/* 510 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 512 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 514 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */

	/* Return value */

/* 516 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 518 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 520 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure LocalOffer */

/* 522 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 524 */	NdrFcLong( 0x0 ),	/* 0 */
/* 528 */	NdrFcShort( 0xe ),	/* 14 */
/* 530 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 532 */	NdrFcShort( 0x0 ),	/* 0 */
/* 534 */	NdrFcShort( 0x8 ),	/* 8 */
/* 536 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter localOffer */

/* 538 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 540 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 542 */	NdrFcShort( 0x42a ),	/* Type Offset=1066 */

	/* Return value */

/* 544 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 546 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 548 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure RemoteOffer */

/* 550 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 552 */	NdrFcLong( 0x0 ),	/* 0 */
/* 556 */	NdrFcShort( 0xf ),	/* 15 */
/* 558 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 560 */	NdrFcShort( 0x0 ),	/* 0 */
/* 562 */	NdrFcShort( 0x8 ),	/* 8 */
/* 564 */	0x5,		/* Oi2 Flags:  srv must size, has return, */
			0x2,		/* 2 */

	/* Parameter localOffer */

/* 566 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 568 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 570 */	NdrFcShort( 0x42a ),	/* Type Offset=1066 */

	/* Return value */

/* 572 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 574 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 576 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure MakeCall */

/* 578 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 580 */	NdrFcLong( 0x0 ),	/* 0 */
/* 584 */	NdrFcShort( 0x10 ),	/* 16 */
/* 586 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 588 */	NdrFcShort( 0x8 ),	/* 8 */
/* 590 */	NdrFcShort( 0x22 ),	/* 34 */
/* 592 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x7,		/* 7 */

	/* Parameter destinationUri */

/* 594 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 596 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 598 */	NdrFcShort( 0x41c ),	/* Type Offset=1052 */

	/* Parameter localOffer */

/* 600 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 602 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 604 */	NdrFcShort( 0x42e ),	/* Type Offset=1070 */

	/* Parameter credentials */

/* 606 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 608 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 610 */	NdrFcShort( 0x440 ),	/* Type Offset=1088 */

	/* Parameter keyValueMap */

/* 612 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 614 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 616 */	NdrFcShort( 0x452 ),	/* Type Offset=1106 */

	/* Parameter ringTimeout */

/* 618 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 620 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 622 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errCode */

/* 624 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 626 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 628 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 630 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 632 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 634 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure ReOffer */

/* 636 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 638 */	NdrFcLong( 0x0 ),	/* 0 */
/* 642 */	NdrFcShort( 0x11 ),	/* 17 */
/* 644 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 646 */	NdrFcShort( 0x8 ),	/* 8 */
/* 648 */	NdrFcShort( 0x22 ),	/* 34 */
/* 650 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x5,		/* 5 */

	/* Parameter localOffer */

/* 652 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 654 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 656 */	NdrFcShort( 0x42e ),	/* Type Offset=1070 */

	/* Parameter keyValueMap */

/* 658 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 660 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 662 */	NdrFcShort( 0x464 ),	/* Type Offset=1124 */

	/* Parameter ringTimeout */

/* 664 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 666 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 668 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errCode */

/* 670 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 672 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 674 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 676 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 678 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 680 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Answer */

/* 682 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 684 */	NdrFcLong( 0x0 ),	/* 0 */
/* 688 */	NdrFcShort( 0x12 ),	/* 18 */
/* 690 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 692 */	NdrFcShort( 0x8 ),	/* 8 */
/* 694 */	NdrFcShort( 0x22 ),	/* 34 */
/* 696 */	0x6,		/* Oi2 Flags:  clt must size, has return, */
			0x5,		/* 5 */

	/* Parameter localOffer */

/* 698 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 700 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 702 */	NdrFcShort( 0x42e ),	/* Type Offset=1070 */

	/* Parameter keyValueMap */

/* 704 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 706 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 708 */	NdrFcShort( 0x464 ),	/* Type Offset=1124 */

	/* Parameter ringTimeout */

/* 710 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 712 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 714 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter errCode */

/* 716 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 718 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 720 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 722 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 724 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 726 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure DtmfBuffer */

/* 728 */	0x33,		/* FC_AUTO_HANDLE */
			0x6c,		/* Old Flags:  object, Oi2 */
/* 730 */	NdrFcLong( 0x0 ),	/* 0 */
/* 734 */	NdrFcShort( 0x13 ),	/* 19 */
/* 736 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 738 */	NdrFcShort( 0x0 ),	/* 0 */
/* 740 */	NdrFcShort( 0x22 ),	/* 34 */
/* 742 */	0x4,		/* Oi2 Flags:  has return, */
			0x2,		/* 2 */

	/* Parameter dtmfBuffer */

/* 744 */	NdrFcShort( 0x2010 ),	/* Flags:  out, srv alloc size=8 */
/* 746 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 748 */	NdrFcShort( 0x426 ),	/* Type Offset=1062 */

	/* Return value */

/* 750 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 752 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 754 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x11, 0x10,	/* FC_RP [pointer_deref] */
/*  4 */	NdrFcShort( 0x2 ),	/* Offset= 2 (6) */
/*  6 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/*  8 */	NdrFcLong( 0x0 ),	/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* 0 */
/* 14 */	NdrFcShort( 0x0 ),	/* 0 */
/* 16 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 18 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 20 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 22 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 24 */	
			0x12, 0x0,	/* FC_UP */
/* 26 */	NdrFcShort( 0x3b8 ),	/* Offset= 952 (978) */
/* 28 */	
			0x2b,		/* FC_NON_ENCAPSULATED_UNION */
			0x9,		/* FC_ULONG */
/* 30 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 32 */	NdrFcShort( 0xfff8 ),	/* -8 */
/* 34 */	NdrFcShort( 0x2 ),	/* Offset= 2 (36) */
/* 36 */	NdrFcShort( 0x10 ),	/* 16 */
/* 38 */	NdrFcShort( 0x2f ),	/* 47 */
/* 40 */	NdrFcLong( 0x14 ),	/* 20 */
/* 44 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 46 */	NdrFcLong( 0x3 ),	/* 3 */
/* 50 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 52 */	NdrFcLong( 0x11 ),	/* 17 */
/* 56 */	NdrFcShort( 0x8001 ),	/* Simple arm type: FC_BYTE */
/* 58 */	NdrFcLong( 0x2 ),	/* 2 */
/* 62 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 64 */	NdrFcLong( 0x4 ),	/* 4 */
/* 68 */	NdrFcShort( 0x800a ),	/* Simple arm type: FC_FLOAT */
/* 70 */	NdrFcLong( 0x5 ),	/* 5 */
/* 74 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 76 */	NdrFcLong( 0xb ),	/* 11 */
/* 80 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 82 */	NdrFcLong( 0xa ),	/* 10 */
/* 86 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 88 */	NdrFcLong( 0x6 ),	/* 6 */
/* 92 */	NdrFcShort( 0xe8 ),	/* Offset= 232 (324) */
/* 94 */	NdrFcLong( 0x7 ),	/* 7 */
/* 98 */	NdrFcShort( 0x800c ),	/* Simple arm type: FC_DOUBLE */
/* 100 */	NdrFcLong( 0x8 ),	/* 8 */
/* 104 */	NdrFcShort( 0xe2 ),	/* Offset= 226 (330) */
/* 106 */	NdrFcLong( 0xd ),	/* 13 */
/* 110 */	NdrFcShort( 0xff98 ),	/* Offset= -104 (6) */
/* 112 */	NdrFcLong( 0x9 ),	/* 9 */
/* 116 */	NdrFcShort( 0xee ),	/* Offset= 238 (354) */
/* 118 */	NdrFcLong( 0x2000 ),	/* 8192 */
/* 122 */	NdrFcShort( 0xfa ),	/* Offset= 250 (372) */
/* 124 */	NdrFcLong( 0x24 ),	/* 36 */
/* 128 */	NdrFcShort( 0x308 ),	/* Offset= 776 (904) */
/* 130 */	NdrFcLong( 0x4024 ),	/* 16420 */
/* 134 */	NdrFcShort( 0x302 ),	/* Offset= 770 (904) */
/* 136 */	NdrFcLong( 0x4011 ),	/* 16401 */
/* 140 */	NdrFcShort( 0x300 ),	/* Offset= 768 (908) */
/* 142 */	NdrFcLong( 0x4002 ),	/* 16386 */
/* 146 */	NdrFcShort( 0x2fe ),	/* Offset= 766 (912) */
/* 148 */	NdrFcLong( 0x4003 ),	/* 16387 */
/* 152 */	NdrFcShort( 0x2fc ),	/* Offset= 764 (916) */
/* 154 */	NdrFcLong( 0x4014 ),	/* 16404 */
/* 158 */	NdrFcShort( 0x2fa ),	/* Offset= 762 (920) */
/* 160 */	NdrFcLong( 0x4004 ),	/* 16388 */
/* 164 */	NdrFcShort( 0x2f8 ),	/* Offset= 760 (924) */
/* 166 */	NdrFcLong( 0x4005 ),	/* 16389 */
/* 170 */	NdrFcShort( 0x2f6 ),	/* Offset= 758 (928) */
/* 172 */	NdrFcLong( 0x400b ),	/* 16395 */
/* 176 */	NdrFcShort( 0x2e0 ),	/* Offset= 736 (912) */
/* 178 */	NdrFcLong( 0x400a ),	/* 16394 */
/* 182 */	NdrFcShort( 0x2de ),	/* Offset= 734 (916) */
/* 184 */	NdrFcLong( 0x4006 ),	/* 16390 */
/* 188 */	NdrFcShort( 0x2e8 ),	/* Offset= 744 (932) */
/* 190 */	NdrFcLong( 0x4007 ),	/* 16391 */
/* 194 */	NdrFcShort( 0x2de ),	/* Offset= 734 (928) */
/* 196 */	NdrFcLong( 0x4008 ),	/* 16392 */
/* 200 */	NdrFcShort( 0x2e0 ),	/* Offset= 736 (936) */
/* 202 */	NdrFcLong( 0x400d ),	/* 16397 */
/* 206 */	NdrFcShort( 0x2de ),	/* Offset= 734 (940) */
/* 208 */	NdrFcLong( 0x4009 ),	/* 16393 */
/* 212 */	NdrFcShort( 0x2dc ),	/* Offset= 732 (944) */
/* 214 */	NdrFcLong( 0x6000 ),	/* 24576 */
/* 218 */	NdrFcShort( 0x2da ),	/* Offset= 730 (948) */
/* 220 */	NdrFcLong( 0x400c ),	/* 16396 */
/* 224 */	NdrFcShort( 0x2d8 ),	/* Offset= 728 (952) */
/* 226 */	NdrFcLong( 0x10 ),	/* 16 */
/* 230 */	NdrFcShort( 0x8002 ),	/* Simple arm type: FC_CHAR */
/* 232 */	NdrFcLong( 0x12 ),	/* 18 */
/* 236 */	NdrFcShort( 0x8006 ),	/* Simple arm type: FC_SHORT */
/* 238 */	NdrFcLong( 0x13 ),	/* 19 */
/* 242 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 244 */	NdrFcLong( 0x15 ),	/* 21 */
/* 248 */	NdrFcShort( 0x800b ),	/* Simple arm type: FC_HYPER */
/* 250 */	NdrFcLong( 0x16 ),	/* 22 */
/* 254 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 256 */	NdrFcLong( 0x17 ),	/* 23 */
/* 260 */	NdrFcShort( 0x8008 ),	/* Simple arm type: FC_LONG */
/* 262 */	NdrFcLong( 0xe ),	/* 14 */
/* 266 */	NdrFcShort( 0x2b6 ),	/* Offset= 694 (960) */
/* 268 */	NdrFcLong( 0x400e ),	/* 16398 */
/* 272 */	NdrFcShort( 0x2ba ),	/* Offset= 698 (970) */
/* 274 */	NdrFcLong( 0x4010 ),	/* 16400 */
/* 278 */	NdrFcShort( 0x2b8 ),	/* Offset= 696 (974) */
/* 280 */	NdrFcLong( 0x4012 ),	/* 16402 */
/* 284 */	NdrFcShort( 0x274 ),	/* Offset= 628 (912) */
/* 286 */	NdrFcLong( 0x4013 ),	/* 16403 */
/* 290 */	NdrFcShort( 0x272 ),	/* Offset= 626 (916) */
/* 292 */	NdrFcLong( 0x4015 ),	/* 16405 */
/* 296 */	NdrFcShort( 0x270 ),	/* Offset= 624 (920) */
/* 298 */	NdrFcLong( 0x4016 ),	/* 16406 */
/* 302 */	NdrFcShort( 0x266 ),	/* Offset= 614 (916) */
/* 304 */	NdrFcLong( 0x4017 ),	/* 16407 */
/* 308 */	NdrFcShort( 0x260 ),	/* Offset= 608 (916) */
/* 310 */	NdrFcLong( 0x0 ),	/* 0 */
/* 314 */	NdrFcShort( 0x0 ),	/* Offset= 0 (314) */
/* 316 */	NdrFcLong( 0x1 ),	/* 1 */
/* 320 */	NdrFcShort( 0x0 ),	/* Offset= 0 (320) */
/* 322 */	NdrFcShort( 0xffff ),	/* Offset= -1 (321) */
/* 324 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 326 */	NdrFcShort( 0x8 ),	/* 8 */
/* 328 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 330 */	
			0x12, 0x0,	/* FC_UP */
/* 332 */	NdrFcShort( 0xc ),	/* Offset= 12 (344) */
/* 334 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 336 */	NdrFcShort( 0x2 ),	/* 2 */
/* 338 */	0x9,		/* Corr desc: FC_ULONG */
			0x0,		/*  */
/* 340 */	NdrFcShort( 0xfffc ),	/* -4 */
/* 342 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 344 */	
			0x17,		/* FC_CSTRUCT */
			0x3,		/* 3 */
/* 346 */	NdrFcShort( 0x8 ),	/* 8 */
/* 348 */	NdrFcShort( 0xfff2 ),	/* Offset= -14 (334) */
/* 350 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 352 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 354 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 356 */	NdrFcLong( 0x20400 ),	/* 132096 */
/* 360 */	NdrFcShort( 0x0 ),	/* 0 */
/* 362 */	NdrFcShort( 0x0 ),	/* 0 */
/* 364 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 366 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 368 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 370 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 372 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 374 */	NdrFcShort( 0x2 ),	/* Offset= 2 (376) */
/* 376 */	
			0x12, 0x0,	/* FC_UP */
/* 378 */	NdrFcShort( 0x1fc ),	/* Offset= 508 (886) */
/* 380 */	
			0x2a,		/* FC_ENCAPSULATED_UNION */
			0x49,		/* 73 */
/* 382 */	NdrFcShort( 0x18 ),	/* 24 */
/* 384 */	NdrFcShort( 0xa ),	/* 10 */
/* 386 */	NdrFcLong( 0x8 ),	/* 8 */
/* 390 */	NdrFcShort( 0x58 ),	/* Offset= 88 (478) */
/* 392 */	NdrFcLong( 0xd ),	/* 13 */
/* 396 */	NdrFcShort( 0x78 ),	/* Offset= 120 (516) */
/* 398 */	NdrFcLong( 0x9 ),	/* 9 */
/* 402 */	NdrFcShort( 0x94 ),	/* Offset= 148 (550) */
/* 404 */	NdrFcLong( 0xc ),	/* 12 */
/* 408 */	NdrFcShort( 0xbc ),	/* Offset= 188 (596) */
/* 410 */	NdrFcLong( 0x24 ),	/* 36 */
/* 414 */	NdrFcShort( 0x114 ),	/* Offset= 276 (690) */
/* 416 */	NdrFcLong( 0x800d ),	/* 32781 */
/* 420 */	NdrFcShort( 0x130 ),	/* Offset= 304 (724) */
/* 422 */	NdrFcLong( 0x10 ),	/* 16 */
/* 426 */	NdrFcShort( 0x148 ),	/* Offset= 328 (754) */
/* 428 */	NdrFcLong( 0x2 ),	/* 2 */
/* 432 */	NdrFcShort( 0x160 ),	/* Offset= 352 (784) */
/* 434 */	NdrFcLong( 0x3 ),	/* 3 */
/* 438 */	NdrFcShort( 0x178 ),	/* Offset= 376 (814) */
/* 440 */	NdrFcLong( 0x14 ),	/* 20 */
/* 444 */	NdrFcShort( 0x190 ),	/* Offset= 400 (844) */
/* 446 */	NdrFcShort( 0xffff ),	/* Offset= -1 (445) */
/* 448 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 450 */	NdrFcShort( 0x4 ),	/* 4 */
/* 452 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 454 */	NdrFcShort( 0x0 ),	/* 0 */
/* 456 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 458 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 460 */	NdrFcShort( 0x4 ),	/* 4 */
/* 462 */	NdrFcShort( 0x0 ),	/* 0 */
/* 464 */	NdrFcShort( 0x1 ),	/* 1 */
/* 466 */	NdrFcShort( 0x0 ),	/* 0 */
/* 468 */	NdrFcShort( 0x0 ),	/* 0 */
/* 470 */	0x12, 0x0,	/* FC_UP */
/* 472 */	NdrFcShort( 0xff80 ),	/* Offset= -128 (344) */
/* 474 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 476 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 478 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 480 */	NdrFcShort( 0x8 ),	/* 8 */
/* 482 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 484 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 486 */	NdrFcShort( 0x4 ),	/* 4 */
/* 488 */	NdrFcShort( 0x4 ),	/* 4 */
/* 490 */	0x11, 0x0,	/* FC_RP */
/* 492 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (448) */
/* 494 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 496 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 498 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 500 */	NdrFcShort( 0x0 ),	/* 0 */
/* 502 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 504 */	NdrFcShort( 0x0 ),	/* 0 */
/* 506 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 510 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 512 */	NdrFcShort( 0xfe06 ),	/* Offset= -506 (6) */
/* 514 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 516 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 518 */	NdrFcShort( 0x8 ),	/* 8 */
/* 520 */	NdrFcShort( 0x0 ),	/* 0 */
/* 522 */	NdrFcShort( 0x6 ),	/* Offset= 6 (528) */
/* 524 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 526 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 528 */	
			0x11, 0x0,	/* FC_RP */
/* 530 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (498) */
/* 532 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 534 */	NdrFcShort( 0x0 ),	/* 0 */
/* 536 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 538 */	NdrFcShort( 0x0 ),	/* 0 */
/* 540 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 544 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 546 */	NdrFcShort( 0xff40 ),	/* Offset= -192 (354) */
/* 548 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 550 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 552 */	NdrFcShort( 0x8 ),	/* 8 */
/* 554 */	NdrFcShort( 0x0 ),	/* 0 */
/* 556 */	NdrFcShort( 0x6 ),	/* Offset= 6 (562) */
/* 558 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 560 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 562 */	
			0x11, 0x0,	/* FC_RP */
/* 564 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (532) */
/* 566 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 568 */	NdrFcShort( 0x4 ),	/* 4 */
/* 570 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 572 */	NdrFcShort( 0x0 ),	/* 0 */
/* 574 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 576 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 578 */	NdrFcShort( 0x4 ),	/* 4 */
/* 580 */	NdrFcShort( 0x0 ),	/* 0 */
/* 582 */	NdrFcShort( 0x1 ),	/* 1 */
/* 584 */	NdrFcShort( 0x0 ),	/* 0 */
/* 586 */	NdrFcShort( 0x0 ),	/* 0 */
/* 588 */	0x12, 0x0,	/* FC_UP */
/* 590 */	NdrFcShort( 0x184 ),	/* Offset= 388 (978) */
/* 592 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 594 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 596 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 598 */	NdrFcShort( 0x8 ),	/* 8 */
/* 600 */	NdrFcShort( 0x0 ),	/* 0 */
/* 602 */	NdrFcShort( 0x6 ),	/* Offset= 6 (608) */
/* 604 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 606 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 608 */	
			0x11, 0x0,	/* FC_RP */
/* 610 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (566) */
/* 612 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 614 */	NdrFcLong( 0x2f ),	/* 47 */
/* 618 */	NdrFcShort( 0x0 ),	/* 0 */
/* 620 */	NdrFcShort( 0x0 ),	/* 0 */
/* 622 */	0xc0,		/* 192 */
			0x0,		/* 0 */
/* 624 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 626 */	0x0,		/* 0 */
			0x0,		/* 0 */
/* 628 */	0x0,		/* 0 */
			0x46,		/* 70 */
/* 630 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 632 */	NdrFcShort( 0x1 ),	/* 1 */
/* 634 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 636 */	NdrFcShort( 0x4 ),	/* 4 */
/* 638 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 640 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 642 */	NdrFcShort( 0x10 ),	/* 16 */
/* 644 */	NdrFcShort( 0x0 ),	/* 0 */
/* 646 */	NdrFcShort( 0xa ),	/* Offset= 10 (656) */
/* 648 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 650 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 652 */	NdrFcShort( 0xffd8 ),	/* Offset= -40 (612) */
/* 654 */	0x36,		/* FC_POINTER */
			0x5b,		/* FC_END */
/* 656 */	
			0x12, 0x0,	/* FC_UP */
/* 658 */	NdrFcShort( 0xffe4 ),	/* Offset= -28 (630) */
/* 660 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 662 */	NdrFcShort( 0x4 ),	/* 4 */
/* 664 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 666 */	NdrFcShort( 0x0 ),	/* 0 */
/* 668 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 670 */	
			0x48,		/* FC_VARIABLE_REPEAT */
			0x49,		/* FC_FIXED_OFFSET */
/* 672 */	NdrFcShort( 0x4 ),	/* 4 */
/* 674 */	NdrFcShort( 0x0 ),	/* 0 */
/* 676 */	NdrFcShort( 0x1 ),	/* 1 */
/* 678 */	NdrFcShort( 0x0 ),	/* 0 */
/* 680 */	NdrFcShort( 0x0 ),	/* 0 */
/* 682 */	0x12, 0x0,	/* FC_UP */
/* 684 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (640) */
/* 686 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 688 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 690 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 692 */	NdrFcShort( 0x8 ),	/* 8 */
/* 694 */	NdrFcShort( 0x0 ),	/* 0 */
/* 696 */	NdrFcShort( 0x6 ),	/* Offset= 6 (702) */
/* 698 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 700 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 702 */	
			0x11, 0x0,	/* FC_RP */
/* 704 */	NdrFcShort( 0xffd4 ),	/* Offset= -44 (660) */
/* 706 */	
			0x1d,		/* FC_SMFARRAY */
			0x0,		/* 0 */
/* 708 */	NdrFcShort( 0x8 ),	/* 8 */
/* 710 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 712 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 714 */	NdrFcShort( 0x10 ),	/* 16 */
/* 716 */	0x8,		/* FC_LONG */
			0x6,		/* FC_SHORT */
/* 718 */	0x6,		/* FC_SHORT */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 720 */	0x0,		/* 0 */
			NdrFcShort( 0xfff1 ),	/* Offset= -15 (706) */
			0x5b,		/* FC_END */
/* 724 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 726 */	NdrFcShort( 0x18 ),	/* 24 */
/* 728 */	NdrFcShort( 0x0 ),	/* 0 */
/* 730 */	NdrFcShort( 0xa ),	/* Offset= 10 (740) */
/* 732 */	0x8,		/* FC_LONG */
			0x36,		/* FC_POINTER */
/* 734 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 736 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (712) */
/* 738 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 740 */	
			0x11, 0x0,	/* FC_RP */
/* 742 */	NdrFcShort( 0xff0c ),	/* Offset= -244 (498) */
/* 744 */	
			0x1b,		/* FC_CARRAY */
			0x0,		/* 0 */
/* 746 */	NdrFcShort( 0x1 ),	/* 1 */
/* 748 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 750 */	NdrFcShort( 0x0 ),	/* 0 */
/* 752 */	0x1,		/* FC_BYTE */
			0x5b,		/* FC_END */
/* 754 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 756 */	NdrFcShort( 0x8 ),	/* 8 */
/* 758 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 760 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 762 */	NdrFcShort( 0x4 ),	/* 4 */
/* 764 */	NdrFcShort( 0x4 ),	/* 4 */
/* 766 */	0x12, 0x0,	/* FC_UP */
/* 768 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (744) */
/* 770 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 772 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 774 */	
			0x1b,		/* FC_CARRAY */
			0x1,		/* 1 */
/* 776 */	NdrFcShort( 0x2 ),	/* 2 */
/* 778 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 780 */	NdrFcShort( 0x0 ),	/* 0 */
/* 782 */	0x6,		/* FC_SHORT */
			0x5b,		/* FC_END */
/* 784 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 786 */	NdrFcShort( 0x8 ),	/* 8 */
/* 788 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 790 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 792 */	NdrFcShort( 0x4 ),	/* 4 */
/* 794 */	NdrFcShort( 0x4 ),	/* 4 */
/* 796 */	0x12, 0x0,	/* FC_UP */
/* 798 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (774) */
/* 800 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 802 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 804 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 806 */	NdrFcShort( 0x4 ),	/* 4 */
/* 808 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 810 */	NdrFcShort( 0x0 ),	/* 0 */
/* 812 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 814 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 816 */	NdrFcShort( 0x8 ),	/* 8 */
/* 818 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 820 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 822 */	NdrFcShort( 0x4 ),	/* 4 */
/* 824 */	NdrFcShort( 0x4 ),	/* 4 */
/* 826 */	0x12, 0x0,	/* FC_UP */
/* 828 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (804) */
/* 830 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 832 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 834 */	
			0x1b,		/* FC_CARRAY */
			0x7,		/* 7 */
/* 836 */	NdrFcShort( 0x8 ),	/* 8 */
/* 838 */	0x19,		/* Corr desc:  field pointer, FC_ULONG */
			0x0,		/*  */
/* 840 */	NdrFcShort( 0x0 ),	/* 0 */
/* 842 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 844 */	
			0x16,		/* FC_PSTRUCT */
			0x3,		/* 3 */
/* 846 */	NdrFcShort( 0x8 ),	/* 8 */
/* 848 */	
			0x4b,		/* FC_PP */
			0x5c,		/* FC_PAD */
/* 850 */	
			0x46,		/* FC_NO_REPEAT */
			0x5c,		/* FC_PAD */
/* 852 */	NdrFcShort( 0x4 ),	/* 4 */
/* 854 */	NdrFcShort( 0x4 ),	/* 4 */
/* 856 */	0x12, 0x0,	/* FC_UP */
/* 858 */	NdrFcShort( 0xffe8 ),	/* Offset= -24 (834) */
/* 860 */	
			0x5b,		/* FC_END */

			0x8,		/* FC_LONG */
/* 862 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/* 864 */	
			0x15,		/* FC_STRUCT */
			0x3,		/* 3 */
/* 866 */	NdrFcShort( 0x8 ),	/* 8 */
/* 868 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 870 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 872 */	
			0x1b,		/* FC_CARRAY */
			0x3,		/* 3 */
/* 874 */	NdrFcShort( 0x8 ),	/* 8 */
/* 876 */	0x7,		/* Corr desc: FC_USHORT */
			0x0,		/*  */
/* 878 */	NdrFcShort( 0xffd8 ),	/* -40 */
/* 880 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 882 */	NdrFcShort( 0xffee ),	/* Offset= -18 (864) */
/* 884 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 886 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 888 */	NdrFcShort( 0x28 ),	/* 40 */
/* 890 */	NdrFcShort( 0xffee ),	/* Offset= -18 (872) */
/* 892 */	NdrFcShort( 0x0 ),	/* Offset= 0 (892) */
/* 894 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 896 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 898 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 900 */	NdrFcShort( 0xfdf8 ),	/* Offset= -520 (380) */
/* 902 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 904 */	
			0x12, 0x0,	/* FC_UP */
/* 906 */	NdrFcShort( 0xfef6 ),	/* Offset= -266 (640) */
/* 908 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 910 */	0x1,		/* FC_BYTE */
			0x5c,		/* FC_PAD */
/* 912 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 914 */	0x6,		/* FC_SHORT */
			0x5c,		/* FC_PAD */
/* 916 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 918 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 920 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 922 */	0xb,		/* FC_HYPER */
			0x5c,		/* FC_PAD */
/* 924 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 926 */	0xa,		/* FC_FLOAT */
			0x5c,		/* FC_PAD */
/* 928 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 930 */	0xc,		/* FC_DOUBLE */
			0x5c,		/* FC_PAD */
/* 932 */	
			0x12, 0x0,	/* FC_UP */
/* 934 */	NdrFcShort( 0xfd9e ),	/* Offset= -610 (324) */
/* 936 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 938 */	NdrFcShort( 0xfda0 ),	/* Offset= -608 (330) */
/* 940 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 942 */	NdrFcShort( 0xfc58 ),	/* Offset= -936 (6) */
/* 944 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 946 */	NdrFcShort( 0xfdb0 ),	/* Offset= -592 (354) */
/* 948 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 950 */	NdrFcShort( 0xfdbe ),	/* Offset= -578 (372) */
/* 952 */	
			0x12, 0x10,	/* FC_UP [pointer_deref] */
/* 954 */	NdrFcShort( 0x2 ),	/* Offset= 2 (956) */
/* 956 */	
			0x12, 0x0,	/* FC_UP */
/* 958 */	NdrFcShort( 0x14 ),	/* Offset= 20 (978) */
/* 960 */	
			0x15,		/* FC_STRUCT */
			0x7,		/* 7 */
/* 962 */	NdrFcShort( 0x10 ),	/* 16 */
/* 964 */	0x6,		/* FC_SHORT */
			0x1,		/* FC_BYTE */
/* 966 */	0x1,		/* FC_BYTE */
			0x8,		/* FC_LONG */
/* 968 */	0xb,		/* FC_HYPER */
			0x5b,		/* FC_END */
/* 970 */	
			0x12, 0x0,	/* FC_UP */
/* 972 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (960) */
/* 974 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 976 */	0x2,		/* FC_CHAR */
			0x5c,		/* FC_PAD */
/* 978 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x7,		/* 7 */
/* 980 */	NdrFcShort( 0x20 ),	/* 32 */
/* 982 */	NdrFcShort( 0x0 ),	/* 0 */
/* 984 */	NdrFcShort( 0x0 ),	/* Offset= 0 (984) */
/* 986 */	0x8,		/* FC_LONG */
			0x8,		/* FC_LONG */
/* 988 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 990 */	0x6,		/* FC_SHORT */
			0x6,		/* FC_SHORT */
/* 992 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 994 */	NdrFcShort( 0xfc3a ),	/* Offset= -966 (28) */
/* 996 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 998 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1000 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1002 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1004 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1006 */	NdrFcShort( 0xfc2a ),	/* Offset= -982 (24) */
/* 1008 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1010 */	NdrFcShort( 0x6 ),	/* Offset= 6 (1016) */
/* 1012 */	
			0x13, 0x0,	/* FC_OP */
/* 1014 */	NdrFcShort( 0xffdc ),	/* Offset= -36 (978) */
/* 1016 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1018 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1020 */	NdrFcShort( 0x10 ),	/* 16 */
/* 1022 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1024 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1012) */
/* 1026 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 1028 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */
/* 1030 */	
			0x11, 0x4,	/* FC_RP [alloced_on_stack] */
/* 1032 */	NdrFcShort( 0x6 ),	/* Offset= 6 (1038) */
/* 1034 */	
			0x13, 0x0,	/* FC_OP */
/* 1036 */	NdrFcShort( 0xfd4c ),	/* Offset= -692 (344) */
/* 1038 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1040 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1042 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1044 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1046 */	NdrFcShort( 0xfff4 ),	/* Offset= -12 (1034) */
/* 1048 */	
			0x11, 0x0,	/* FC_RP */
/* 1050 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1052) */
/* 1052 */	0xb4,		/* FC_USER_MARSHAL */
			0x83,		/* 131 */
/* 1054 */	NdrFcShort( 0x1 ),	/* 1 */
/* 1056 */	NdrFcShort( 0x4 ),	/* 4 */
/* 1058 */	NdrFcShort( 0x0 ),	/* 0 */
/* 1060 */	NdrFcShort( 0xfd26 ),	/* Offset= -730 (330) */
/* 1062 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 1064 */	0xd,		/* FC_ENUM16 */
			0x5c,		/* FC_PAD */
/* 1066 */	
			0x11, 0x10,	/* FC_RP [pointer_deref] */
/* 1068 */	NdrFcShort( 0x2 ),	/* Offset= 2 (1070) */
/* 1070 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1072 */	NdrFcLong( 0x19e2d99b ),	/* 434297243 */
/* 1076 */	NdrFcShort( 0x3fe2 ),	/* 16354 */
/* 1078 */	NdrFcShort( 0x40bb ),	/* 16571 */
/* 1080 */	0xbf,		/* 191 */
			0x2e,		/* 46 */
/* 1082 */	0x8,		/* 8 */
			0xe8,		/* 232 */
/* 1084 */	0x9a,		/* 154 */
			0x30,		/* 48 */
/* 1086 */	0x38,		/* 56 */
			0xc9,		/* 201 */
/* 1088 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1090 */	NdrFcLong( 0x90c8f27f ),	/* -1865878913 */
/* 1094 */	NdrFcShort( 0xc8af ),	/* -14161 */
/* 1096 */	NdrFcShort( 0x4cf3 ),	/* 19699 */
/* 1098 */	0xbc,		/* 188 */
			0xf4,		/* 244 */
/* 1100 */	0x9f,		/* 159 */
			0x83,		/* 131 */
/* 1102 */	0x8a,		/* 138 */
			0xc4,		/* 196 */
/* 1104 */	0x2a,		/* 42 */
			0x17,		/* 23 */
/* 1106 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1108 */	NdrFcLong( 0xe9a35b70 ),	/* -375170192 */
/* 1112 */	NdrFcShort( 0x8738 ),	/* -30920 */
/* 1114 */	NdrFcShort( 0x4f38 ),	/* 20280 */
/* 1116 */	0xa6,		/* 166 */
			0x85,		/* 133 */
/* 1118 */	0xec,		/* 236 */
			0xa5,		/* 165 */
/* 1120 */	0xb2,		/* 178 */
			0xcb,		/* 203 */
/* 1122 */	0xca,		/* 202 */
			0x59,		/* 89 */
/* 1124 */	
			0x2f,		/* FC_IP */
			0x5a,		/* FC_CONSTANT_IID */
/* 1126 */	NdrFcLong( 0xe9a35b70 ),	/* -375170192 */
/* 1130 */	NdrFcShort( 0x8738 ),	/* -30920 */
/* 1132 */	NdrFcShort( 0x4f38 ),	/* 20280 */
/* 1134 */	0xa6,		/* 166 */
			0x85,		/* 133 */
/* 1136 */	0xec,		/* 236 */
			0xa5,		/* 165 */
/* 1138 */	0xb2,		/* 178 */
			0xcb,		/* 203 */
/* 1140 */	0xca,		/* 202 */
			0x59,		/* 89 */

			0x0
        }
    };

static const USER_MARSHAL_ROUTINE_QUADRUPLE UserMarshalRoutines[ WIRE_MARSHAL_TABLE_SIZE ] = 
        {
            
            {
            VARIANT_UserSize
            ,VARIANT_UserMarshal
            ,VARIANT_UserUnmarshal
            ,VARIANT_UserFree
            },
            {
            BSTR_UserSize
            ,BSTR_UserMarshal
            ,BSTR_UserUnmarshal
            ,BSTR_UserFree
            }

        };



/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IComMap, ver. 0.0,
   GUID={0xE9A35B70,0x8738,0x4F38,{0xA6,0x85,0xEC,0xA5,0xB2,0xCB,0xCA,0x59}} */

#pragma code_seg(".orpc")
static const unsigned short IComMap_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0,
    28,
    62,
    90,
    124,
    152
    };

static const MIDL_STUBLESS_PROXY_INFO IComMap_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IComMap_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IComMap_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IComMap_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(13) _IComMapProxyVtbl = 
{
    &IComMap_ProxyInfo,
    &IID_IComMap,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IComMap::get__NewEnum */ ,
    (void *) (INT_PTR) -1 /* IComMap::get_Item */ ,
    (void *) (INT_PTR) -1 /* IComMap::get_Count */ ,
    (void *) (INT_PTR) -1 /* IComMap::Add */ ,
    (void *) (INT_PTR) -1 /* IComMap::Remove */ ,
    (void *) (INT_PTR) -1 /* IComMap::Clear */
};


static const PRPC_STUB_FUNCTION IComMap_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IComMapStubVtbl =
{
    &IID_IComMap,
    &IComMap_ServerInfo,
    13,
    &IComMap_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IComIVRWORX, ver. 0.0,
   GUID={0xBABCF77D,0x476B,0x4E79,{0x8F,0xC0,0x8E,0xA1,0x4F,0xB6,0xAF,0x8D}} */

#pragma code_seg(".orpc")
static const unsigned short IComIVRWORX_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    0
    };

static const MIDL_STUBLESS_PROXY_INFO IComIVRWORX_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IComIVRWORX_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IComIVRWORX_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IComIVRWORX_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(7) _IComIVRWORXProxyVtbl = 
{
    0,
    &IID_IComIVRWORX,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


static const PRPC_STUB_FUNCTION IComIVRWORX_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IComIVRWORXStubVtbl =
{
    &IID_IComIVRWORX,
    &IComIVRWORX_ServerInfo,
    7,
    &IComIVRWORX_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IComCredentials, ver. 0.0,
   GUID={0x90C8F27F,0xC8AF,0x4CF3,{0xBC,0xF4,0x9F,0x83,0x8A,0xC4,0x2A,0x17}} */

#pragma code_seg(".orpc")
static const unsigned short IComCredentials_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    174,
    202,
    230,
    258,
    286,
    314
    };

static const MIDL_STUBLESS_PROXY_INFO IComCredentials_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IComCredentials_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IComCredentials_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IComCredentials_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(13) _IComCredentialsProxyVtbl = 
{
    &IComCredentials_ProxyInfo,
    &IID_IComCredentials,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IComCredentials::get_User */ ,
    (void *) (INT_PTR) -1 /* IComCredentials::put_User */ ,
    (void *) (INT_PTR) -1 /* IComCredentials::get_Password */ ,
    (void *) (INT_PTR) -1 /* IComCredentials::put_Password */ ,
    (void *) (INT_PTR) -1 /* IComCredentials::get_Realm */ ,
    (void *) (INT_PTR) -1 /* IComCredentials::put_Realm */
};


static const PRPC_STUB_FUNCTION IComCredentials_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IComCredentialsStubVtbl =
{
    &IID_IComCredentials,
    &IComCredentials_ServerInfo,
    13,
    &IComCredentials_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IComAbstractOffer, ver. 0.0,
   GUID={0x19E2D99B,0x3FE2,0x40BB,{0xBF,0x2E,0x08,0xE8,0x9A,0x30,0x38,0xC9}} */

#pragma code_seg(".orpc")
static const unsigned short IComAbstractOffer_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    174,
    202,
    230,
    258
    };

static const MIDL_STUBLESS_PROXY_INFO IComAbstractOffer_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IComAbstractOffer_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IComAbstractOffer_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IComAbstractOffer_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(11) _IComAbstractOfferProxyVtbl = 
{
    &IComAbstractOffer_ProxyInfo,
    &IID_IComAbstractOffer,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IComAbstractOffer::get_Type */ ,
    (void *) (INT_PTR) -1 /* IComAbstractOffer::put_Type */ ,
    (void *) (INT_PTR) -1 /* IComAbstractOffer::get_Body */ ,
    (void *) (INT_PTR) -1 /* IComAbstractOffer::put_Body */
};


static const PRPC_STUB_FUNCTION IComAbstractOffer_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IComAbstractOfferStubVtbl =
{
    &IID_IComAbstractOffer,
    &IComAbstractOffer_ServerInfo,
    11,
    &IComAbstractOffer_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Standard interface: __MIDL_itf_iw_cominterop_0265, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}} */


/* Object interface: IComSIPCall, ver. 0.0,
   GUID={0x533A9932,0xE0F0,0x4B30,{0x86,0xCE,0x5B,0x2B,0xA3,0x65,0x3E,0x95}} */

#pragma code_seg(".orpc")
static const unsigned short IComSIPCall_FormatStringOffsetTable[] =
    {
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    (unsigned short) -1,
    342,
    382,
    410,
    438,
    472,
    314,
    494,
    522,
    550,
    578,
    636,
    682,
    728
    };

static const MIDL_STUBLESS_PROXY_INFO IComSIPCall_ProxyInfo =
    {
    &Object_StubDesc,
    __MIDL_ProcFormatString.Format,
    &IComSIPCall_FormatStringOffsetTable[-3],
    0,
    0,
    0
    };


static const MIDL_SERVER_INFO IComSIPCall_ServerInfo = 
    {
    &Object_StubDesc,
    0,
    __MIDL_ProcFormatString.Format,
    &IComSIPCall_FormatStringOffsetTable[-3],
    0,
    0,
    0,
    0};
CINTERFACE_PROXY_VTABLE(20) _IComSIPCallProxyVtbl = 
{
    &IComSIPCall_ProxyInfo,
    &IID_IComSIPCall,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *) (INT_PTR) -1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::WaitForDtmf */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::RejectCall */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::HangupCall */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::BlindXfer */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::WaitTillHangup */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::Dnis */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::Ani */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::LocalOffer */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::RemoteOffer */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::MakeCall */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::ReOffer */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::Answer */ ,
    (void *) (INT_PTR) -1 /* IComSIPCall::DtmfBuffer */
};


static const PRPC_STUB_FUNCTION IComSIPCall_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2,
    NdrStubCall2
};

CInterfaceStubVtbl _IComSIPCallStubVtbl =
{
    &IID_IComSIPCall,
    &IComSIPCall_ServerInfo,
    20,
    &IComSIPCall_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x600016e, /* MIDL Version 6.0.366 */
    0,
    UserMarshalRoutines,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0   /* Reserved5 */
    };

const CInterfaceProxyVtbl * _iw_cominterop_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IComSIPCallProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IComMapProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IComIVRWORXProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IComCredentialsProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IComAbstractOfferProxyVtbl,
    0
};

const CInterfaceStubVtbl * _iw_cominterop_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IComSIPCallStubVtbl,
    ( CInterfaceStubVtbl *) &_IComMapStubVtbl,
    ( CInterfaceStubVtbl *) &_IComIVRWORXStubVtbl,
    ( CInterfaceStubVtbl *) &_IComCredentialsStubVtbl,
    ( CInterfaceStubVtbl *) &_IComAbstractOfferStubVtbl,
    0
};

PCInterfaceName const _iw_cominterop_InterfaceNamesList[] = 
{
    "IComSIPCall",
    "IComMap",
    "IComIVRWORX",
    "IComCredentials",
    "IComAbstractOffer",
    0
};

const IID *  _iw_cominterop_BaseIIDList[] = 
{
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    &IID_IDispatch,
    0
};


#define _iw_cominterop_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _iw_cominterop, pIID, n)

int __stdcall _iw_cominterop_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _iw_cominterop, 5, 4 )
    IID_BS_LOOKUP_NEXT_TEST( _iw_cominterop, 2 )
    IID_BS_LOOKUP_NEXT_TEST( _iw_cominterop, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _iw_cominterop, 5, *pIndex )
    
}

const ExtendedProxyFileInfo iw_cominterop_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _iw_cominterop_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _iw_cominterop_StubVtblList,
    (const PCInterfaceName * ) & _iw_cominterop_InterfaceNamesList,
    (const IID ** ) & _iw_cominterop_BaseIIDList,
    & _iw_cominterop_IID_Lookup, 
    5,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/


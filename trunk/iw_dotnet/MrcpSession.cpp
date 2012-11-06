/*
*	The Altalena Project File
*	Copyright (C) 2009  Boris Ouretskey
*
*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "StdAfx.h"
#include "MrcpSession.h"


using namespace System;
using namespace System::Runtime::InteropServices;

using namespace ivrworx;
using namespace interop;

namespace ivrworx
{
namespace interop
{

MrcpSession::MrcpSession (IN IvrWORX ^threadCtx)
{
	HandleId service_handle_id = IW_UNDEFINED;
	service_handle_id  = threadCtx->GetServiceHandle(gcnew String("ivr/mrcp_service"));

	if (service_handle_id ==  IW_UNDEFINED) 
	{
		throw gcnew Exception("Service not found");
	}

	_impl = new ivrworx::MrcpSession(*threadCtx->Forking,service_handle_id);

}

MrcpSession::~MrcpSession (void)
{
	if (_impl)
		_impl->TearDown();
}



ApiErrorCode	
MrcpSession::Allocate(IN MrcpResource rsc)
{
	ivrworx::ApiErrorCode res = 
		_impl->Allocate((ivrworx::MrcpResource)rsc);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode	
MrcpSession::Allocate(
	IN MrcpResource rsc,
	IN AbstractOffer ^localOffer, 
	IN Int32 timeout)
{
	
	DECLARE_OFFER_FROM_MANAGED(localOffer_, localOffer);

	ivrworx::ApiErrorCode res = 
		_impl->Allocate((ivrworx::MrcpResource)rsc,localOffer_,MilliSeconds(timeout));

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode	
MrcpSession::ModifySession(
	IN AbstractOffer ^remoteOffer, 
	IN Int32 timeout)
{
	DECLARE_OFFER_FROM_MANAGED(remoteOffer_, remoteOffer);

	ivrworx::ApiErrorCode res = 
		_impl->ModifySession(remoteOffer_,MilliSeconds(timeout));

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode	
MrcpSession::StopSpeak()
{
	ivrworx::ApiErrorCode res = 
		_impl->StopSpeak();

	return ivrworx::interop::ApiErrorCode(res);


}

ApiErrorCode MrcpSession::Speak(
	IN  MrcpParams ^params,
	IN  String ^body,
	IN  bool sync )
{
	DECLARE_MAPOFANY_FROM_MANAGED(params_,params);
	
	ivrworx::ApiErrorCode res = 
		_impl->Speak(params_,MarshalToString(body),sync);

	return ivrworx::interop::ApiErrorCode(res);

}

ApiErrorCode MrcpSession::SimpleSpeak(
							   IN  MrcpParams ^params,
							   IN  String ^sentence,
							   IN  bool sync )
{
	

	if (params == nullptr)
		params = gcnew MrcpParams();

	String ^key = "content_type";
	String ^value  = "application/synthesis+ssml";
	params[key] = value;

																	
	String ^body =   
		"<?xml version=\"1.0\"?> \
		   <speak version=\"1.0\" xml:lang=\"en-US\" xmlns=\"http://www.w3.org/2001/10/synthesis\"> \
		     <p> \
		       <s>" + sentence + "</s>	\
		     </p> \
		</speak>";

	ApiErrorCode res = 
		this->Speak(
			params, 
			body, 
			true);

	return res;

}


ApiErrorCode MrcpSession::Recognize(
	IN  MrcpParams ^p,
	IN  String ^body,
	IN  Int32 timeout, 
	IN  bool sync, 
	[Out] String^%answer)
{

	DECLARE_MAPOFANY_FROM_MANAGED(p_,p);

	string answer_;

	ivrworx::ApiErrorCode res = 
 		_impl->Recognize(p_,MarshalToString(body),MilliSeconds(timeout),sync,answer_);

	answer = gcnew String(answer_.c_str()); 

	return ivrworx::interop::ApiErrorCode(res);

}

 ApiErrorCode MrcpSession::WaitForRecogResult(
	IN  Int32 timeout,
	[Out] String^%answer)
 {
	 string answer_;

	 ivrworx::ApiErrorCode res = 
		 _impl->WaitForRecogResult(MilliSeconds(timeout), answer_);

	 answer = gcnew String(answer_.c_str()); 

	 return ivrworx::interop::ApiErrorCode(res);


 }

ApiErrorCode 
MrcpSession::DefineGrammar(
	IN  MrcpParams ^p,
	IN  String ^body,
	IN  Int32 timeout)
{
	DECLARE_MAPOFANY_FROM_MANAGED(p_,p);


	ivrworx::ApiErrorCode res = 
		_impl->DefineGrammar(p_,
		MarshalToString(body),
		MilliSeconds(timeout));


	return ivrworx::interop::ApiErrorCode(res);
	
}

void 
MrcpSession::TearDown()
{
	_impl->TearDown();

}

AbstractOffer^ 
MrcpSession::RemoteOffer(MrcpResource rsc)
{
	return gcnew AbstractOffer(_impl->RemoteOffer((ivrworx::MrcpResource)rsc));
}

AbstractOffer^ 
MrcpSession::LocalOffer(MrcpResource rsc)
{
	return gcnew AbstractOffer(_impl->LocalOffer((ivrworx::MrcpResource)rsc));

}



}
}
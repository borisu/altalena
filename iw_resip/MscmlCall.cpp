#include "StdAfx.h"
#include "MscmlCall.h"

namespace ivrworx
{

MscmlCall::MscmlCall(IN ScopedForking &forking, IN HandleId handle_id):
SipMediaCall(forking,handle_id)
{
	OfferType("mediaservercontrol+xml");
}

MscmlCall::~MscmlCall(void)
{
}



}

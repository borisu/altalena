#include "StdAfx.h"
#include "ResipMscmlCall.h"

namespace ivrworx
{

ResipMscmlCall::ResipMscmlCall(IN ScopedForking &forking, IN HandleId handle_id):
ResipMediaCall(forking,handle_id)
{
	OfferType("mediaservercontrol+xml");
}

ResipMscmlCall::~ResipMscmlCall(void)
{
}



}

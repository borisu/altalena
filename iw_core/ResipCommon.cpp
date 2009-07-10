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

#include "stdafx.h"
#include "ResipCommon.h"
#include "IwBase.h"


using namespace std;
using namespace resip;

namespace ivrworx
{


	ostream& 
	operator << (ostream &ostream, const Data &data)
	{
		return ostream << data.c_str();
	}

	ostream& 
	operator<<(std::ostream& strm, 
		const Message& msg)
	{
#pragma TODO("Add real output")
		return strm;
	}

	IwStackHandle 
	GenerateSipHandle()
	{
		static volatile int index = 10000;
		return ::InterlockedIncrement((LONG*)&index);
	};

	SipDialogContext::SipDialogContext():
	stack_handle(IW_UNDEFINED)
	{

	}

	CnxInfo 
	ExtractCnxInfo(const SdpContents& sdp)
	{
		const SdpContents::Session &s = sdp.session();
		const Data &addr_data = s.connection().getAddress();
		const string addr = addr_data.c_str();

		const SdpContents::Session::Medium &medium = s.media().front();
		int port = medium.port();

		return CnxInfo(addr,port);
	}

	

}


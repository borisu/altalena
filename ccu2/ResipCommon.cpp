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
#include "Ccu.h"



using namespace std;
using namespace resip;

wostream& 
operator << (wostream &ostream, const Data &data)
{
	return ostream << data.c_str();
}

wostream& 
operator<<(std::wostream& strm, 
				  const Message& msg)
{
#pragma TODO("Efficiency warning")
	stringstream  stream;
	throw;
// 	ResipFastOStream os(stream);
// 	msg.encode(&os);
	return strm << StringToWString(stream.str());
}

CcuStackHandle 
GenerateSipHandle()
{
	static int index = 10000;
	return index ++;
};

SipDialogContext::SipDialogContext():
stack_handle(CCU_UNDEFINED)
{

}

CcuMediaData 
CreateMediaData(const SdpContents& sdp)
{
	const SdpContents::Session &s = sdp.session();
	const Data &addr_data = s.connection().getAddress();
	const string addr = addr_data.c_str();

	const SdpContents::Session::Medium &medium = s.media().front();
	int port = medium.port();

	return CcuMediaData(addr,port);
}


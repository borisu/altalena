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
#include "UASDialogUsageManagerTest.h"
#include "UASDialogUsageManager.h"
#include "MockResipStack.h"
#include "ProcSipStack.h"
#include "ProcTestStub.h"

using namespace ivrworx;

UASDialogUsageManagerTest::UASDialogUsageManagerTest(void)
{
}

UASDialogUsageManagerTest::~UASDialogUsageManagerTest(void)
{
}

void
UASDialogUsageManagerTest::test()
{

	CcuConfiguration conf;

	conf.AddCodec(new MediaFormat(L"PCMU",8000,0));
	conf.AddCodec(new MediaFormat(L"PCMA",8000,8));
	

	MockResipStack mock_stack;
	CcuHandlesMap map;

	DECLARE_NAMED_HANDLE_PAIR (test);
	ProcTestStub stub(test);

	UASDialogUsageManager mngr(conf, mock_stack,map,stub);

	CnxInfo info("1.2.3.4",6555);

	string sdp = mngr.CreateSdp(info, MediaFormat::PCMA);

	// version depends on time so look only 
	// for substring
	char * buf=
		//"v=0\r\n"
		//"o=ivrworx 948688500 948688375 IN IP4 1.2.3.4\r\n"
		"s=myivr\r\n"
		"c=IN IP4 1.2.3.4\r\n"
		"t=0 0\r\n"
		"m=audio 6555 RTP/AVP 8 0\r\n"
		"a=rtpmap:8 PCMA/8000\r\n"
		"a=rtpmap:0 PCMU/8000\r\n"
		"\r\n";

	assert(sdp.find(buf) != string::npos);

	

}

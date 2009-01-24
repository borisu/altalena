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

	conf.AddCodec(new IxCodec(L"pcma",0,8000));
	conf.AddCodec(new IxCodec(L"pcmu",8,8000));
	

	MockResipStack mock_stack;
	CcuHandlesMap map;

	DECLARE_NAMED_HANDLE_PAIR (test);
	ProcTestStub stub(test);

	UASDialogUsageManager mngr(conf, mock_stack,map,stub);

	CnxInfo info("1.2.3.4",6555);

	string sdp = mngr.CreateSdp(info);

}

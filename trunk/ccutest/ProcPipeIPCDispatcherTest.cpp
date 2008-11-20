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
#include "ProcTestStub.h"
#include "ProcPipeIPCDispatcherTest.h"
#include "ProcPipeIPCDispatcher.h"

ProcPipeIPCDispatcherTest::ProcPipeIPCDispatcherTest(void)
{
}

ProcPipeIPCDispatcherTest::~ProcPipeIPCDispatcherTest(void)
{
}

void
ProcPipeIPCDispatcherTest::test()
{
	START_FORKING_REGION;
	DECLARE_NAMED_HANDLE_PAIR(test_pair);

	ProcIPCTester *p = new ProcIPCTester(test_pair);

	FORK_IN_THIS_THREAD(p);

	END_FORKING_REGION;

}


ProcIPCTester::ProcIPCTester(LpHandlePair pair):
LightweightProcess(pair,__FUNCTIONW__)
{

}

void
ProcIPCTester::real_run()
{
	
	START_FORKING_REGION;

	DECLARE_NAMED_HANDLE_PAIR(server_pair);
	ProcPipeIPCDispatcher *server = new ProcPipeIPCDispatcher(
		server_pair, 
		L"\\\\.\\pipe\\TEST_SERVER");
	

	
	DECLARE_NAMED_HANDLE_PAIR(client_pair);
	ProcPipeIPCDispatcher *client = new ProcPipeIPCDispatcher(
		client_pair, 
		L"\\\\.\\pipe\\TEST_CLIENT");
	

	DECLARE_NAMED_HANDLE_PAIR(test_pair);
	ProcTestStub *test = new ProcTestStub(
		test_pair);

	BucketPtr testBucket =test->_bucket;
	test->_exitMsg = CCU_MSG_PONG;



	FORK(server);
 	FORK(client);
 	FORK(test);

	CcuMsgPing *ping = new CcuMsgPing();
	ping->dest.proc_id = test->ProcessId();
	ping->dest.queue_path = L"\\\\.\\pipe\\TEST_SERVER";
	ping->source.proc_id = test->ProcessId();

	client_pair.inbound->Send(ping);

	LightweightProcess::Join(testBucket);
	Shutdown(Seconds(1), server_pair); 
    Shutdown(Seconds(1), client_pair);


	END_FORKING_REGION;

	LogDebug("Success!!!");

}

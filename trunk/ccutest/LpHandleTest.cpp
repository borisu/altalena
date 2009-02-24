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
#include "LpHandleTest.h"
#include "LpHandle.h"
#include "LightweightProcess.h"

LpHandleTest::LpHandleTest(void)
{
}

LpHandleTest::~LpHandleTest(void)
{
}

void
LpHandleTest::test()
{
	testSelectFromChannels();
	testPoison();
	testMaxMessages();
	
}

void
LpHandleTest::testPoison()
{
	LpHandle h;

	h.Poison();

	try
	{
		h.Send(new CcuMsgAck());
		assert(0);
	}
	catch (std::exception e)
	{
		
	}

}

void
LpHandleTest::testMaxMessages()
{
	LpHandle h;

	for (int i=0; i < CCU_MAX_MESSAGES_IN_QUEUE ; i++)
	{
		printf("inserting message #[%d]",i);
		h.Send(new CcuMsgAck());
	}

	try 
	{
		h.Send(new CcuMsgAck());
		assert(false);
	} catch (...)
	{

	}
#pragma TODO("Make overflowing revertible (now it seems like it poisond the channel")
	

}

void
LpHandleTest::testSelectFromChannels()
{

	START_FORKING_REGION;

	HandlesList list;

	DECLARE_NAMED_HANDLE(h1);
	DECLARE_NAMED_HANDLE(h2);
	DECLARE_NAMED_HANDLE(h3);
	
	list.push_back(h1); int h1_index = 0;
	list.push_back(h2); int h2_index = 1;
	list.push_back(h3); int h3_index = 2;
	
	int index = IW_UNDEFINED;
	IwMessagePtr event;

	// wait with time 0
	h1->Send(new CcuMsgAck());
	SelectFromChannels(
		list,
		Seconds(0), 
		index, 
		event);
	assert(index==0);
	assert(event->message_id == CCU_MSG_ACK);

	
	// simple wait
	DECLARE_NAMED_HANDLE_PAIR(script_pair);
	FORK_IN_THIS_THREAD(
		new ProcVoidFuncRunner<LpHandleTest>(
		script_pair,
		bind<void>(&LpHandleTest::sleep_and_send, _1, h2),
		this,
		L"Message Sender"));
	
	
	SleepFor(Seconds(1));
	// simple wait
	SelectFromChannels(
		list,
		Seconds(10), 
		index, 
		event);
	assert(index==1);
	assert(event->message_id == CCU_MSG_ACK);
	

	
	// timeout
	IxApiErrorCode res = SelectFromChannels(
		list,
		Seconds(0), 
		index, 
		event);

	assert(res==CCU_API_TIMEOUT);
	assert(index==-1);

	END_FORKING_REGION;

}

void 
LpHandleTest::sleep_and_send(LpHandlePtr h)
{
	h->Send(new CcuMsgAck());
}
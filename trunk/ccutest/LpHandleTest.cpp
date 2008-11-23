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

LpHandleTest::LpHandleTest(void)
{
}

LpHandleTest::~LpHandleTest(void)
{
}

void
LpHandleTest::test()
{
	testPoison();
	testMaxMessages();
}

void
LpHandleTest::testPoison()
{
	LpHandle h;

	h.Poison();

	assert(CCU_FAILURE(h.Send(new CcuMsgAck())));
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

	assert(CCU_FAILURE(h.Send(new CcuMsgAck())));

	CcuApiErrorCode res = CCU_API_SUCCESS;
	h.Wait(Seconds(1),res);
	assert(CCU_SUCCESS(h.Send(new CcuMsgAck())));



}
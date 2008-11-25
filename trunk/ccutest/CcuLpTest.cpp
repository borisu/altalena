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
#include "CcuLpTest.h"
#include "LightweightProcess.h"




class ProcDoomedToBeTerminated:
	public LightweightProcess
{
public:

	ProcDoomedToBeTerminated(LpHandlePair pair):LightweightProcess(pair,__FUNCTIONW__){};

	void real_run()
	{
		
		DECLARE_NAMED_HANDLE_PAIR(pair);

		// try to shutdown non existing process
		Shutdown(Seconds(6000), pair);
		
	}

};

CcuLightweightProcessTest::CcuLightweightProcessTest(void)
{
}

CcuLightweightProcessTest::~CcuLightweightProcessTest(void)
{
}

void
CcuLightweightProcessTest::test_txn_termination()
{
	DECLARE_NAMED_HANDLE_PAIR(pair);

	START_FORKING_REGION;

	LightweightProcess *f =  new ProcDoomedToBeTerminated(pair);

	FORK(f);

	f->TerminatePendingTransaction(std::exception("cool"));


	END_FORKING_REGION;
}

CcuApiErrorCode
CcuLightweightProcessTest::throw_std()
{
	throw std::exception();

	return CCU_API_FAILURE;
}

void
CcuLightweightProcessTest::test_func_runner()
{

		
	CcuApiErrorCode res = CCU_API_FAILURE;

	ProcFuncRunner<CcuApiErrorCode,CcuLightweightProcessTest> 
			*test_proc = new ProcFuncRunner<CcuApiErrorCode,CcuLightweightProcessTest>(		
				bind<CcuApiErrorCode>(&CcuLightweightProcessTest::throw_std, _1),
				this,
				res,
				__FUNCTIONW__);


	Run(test_proc);

	assert(res == CCU_API_FAILURE);


}

void
CcuLightweightProcessTest::test()
{
	test_func_runner();
	test_txn_termination();

}

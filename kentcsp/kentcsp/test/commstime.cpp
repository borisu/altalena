/*
*	The Kent C++CSP Library
*	Copyright (C) 2002-2008 Neil Brown
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

// Change to cppcsp/cppcsp.h if not building as part of the source distribution
#include "src/cppcsp.h"
#include "src/common/basic.h"
#include <iostream>

using namespace csp;

#define LOOP_WARMUP (1000)
#define LOOP_AMOUNT (1000000) /*1 million*/

template <class DATA_TYPE>
class Consume : public CSProcess
{
private:
	Chanin<DATA_TYPE> channelIn;
	DATA_TYPE t;
protected:
	void run()
	{
		long i;

		//Take in the first few to rule out the times for starting up:
		for (i = 0;i < LOOP_WARMUP;i++)
		{
			channelIn >> t;
		}

		Time tstart,tend;
		CurrentTime(&tstart);

		for (i = 0;i < LOOP_AMOUNT;i++)
		{			
			channelIn >> t;
		}

		CurrentTime(&tend);
		tend -= tstart;
		
		channelIn.poison();

		std::cout << "Time per iteration: " << static_cast<double>(GetMilliSeconds(tend))/(LOOP_AMOUNT/1000.0) << " microseconds" << std::endl;
	}
public:
	Consume(csp::Chanin<DATA_TYPE> chIn)
		:	CSProcess(65536),channelIn(chIn)
	{
	};	
};

//Change this to try 64-bit commstime
#define INT int

int main(int, char**)
{
	Start_CPPCSP();

	One2OneChannel<INT> cha,chb,chc,chd;

	std::cout << "Timing using sequential delta, one OS thread:" << std::endl;

	RunInThisThread( InParallelOneThread
		(new csp::common::Prefix<INT>(chc.reader(),cha.writer(),0))
		(new csp::common::SeqDelta<INT>(cha.reader(),chd.writer(),chb.writer()))
		(new csp::common::Successor<INT>(chb.reader(),chc.writer()))
		(new Consume<INT>(chd.reader()))
	  );

	One2OneChannel<INT> cha1,chb1,chc1,chd1;

	std::cout << "Timing using sequential delta, OS thread per process:" << std::endl;

	Run( InParallel
		(new csp::common::Prefix<INT>(chc1.reader(),cha1.writer(),0))
		(new csp::common::SeqDelta<INT>(cha1.reader(),chd1.writer(),chb1.writer()))
		(new csp::common::Successor<INT>(chb1.reader(),chc1.writer()))
		(new Consume<INT>(chd1.reader()))
	  );

	One2OneChannel<INT> cha2,chb2,chc2,chd2;

	std::cout << "Timing using parallel delta, one OS thread:" << std::endl;

	RunInThisThread( InParallelOneThread
		(new csp::common::Prefix<INT>(chc2.reader(),cha2.writer(),0))
		(new csp::common::Delta<INT>(cha2.reader(),chd2.writer(),chb2.writer()))
		(new csp::common::Successor<INT>(chb2.reader(),chc2.writer()))
		(new Consume<INT>(chd2.reader()))
	  );

	One2OneChannel<INT> cha3,chb3,chc3,chd3;

	std::cout << "Timing using parallel delta, OS thread per process:" << std::endl;

	Run( InParallel
		(new csp::common::Prefix<INT>(chc3.reader(),cha3.writer(),0))
		(new csp::common::Delta<INT>(cha3.reader(),chd3.writer(),chb3.writer()))
		(new csp::common::Successor<INT>(chb3.reader(),chc3.writer()))
		(new Consume<INT>(chd3.reader()))
	  );

	End_CPPCSP();
}

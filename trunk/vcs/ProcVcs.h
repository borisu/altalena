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

#pragma once

#include "LightweightProcess.h"
#include "CcuConfiguration.h"
#include "Call.h"

using namespace std;

class ProcVcs :
	public LightweightProcess
{

public:

#pragma TODO("Leave only ctor that receives configuration object as a parameter")

	ProcVcs(IN LpHandlePair pair, IN CnxInfo sip_stack_media);

	ProcVcs(IN LpHandlePair pair, IN CcuConfiguration &conf);

	virtual void real_run();

	virtual ~ProcVcs(void);

protected:

	BOOL ProcessStackMessage(
		IN CcuMsgPtr event 
		);

	BOOL ProcessInboundMessage(
		IN CcuMsgPtr event,
		IN ScopedForking &forking
		);

private:

	CnxInfo _sipStackData;

	CcuConfiguration *_conf;

};

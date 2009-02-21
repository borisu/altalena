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
#include "PortManager.h"
#include "Ccu.h"
#include "CcuLogger.h"

using namespace std;

namespace ivrworx
{

#define CCU_MAX_PORT_RANGE 65000

PortManager::PortManager(int abs_top, int abs_bottom)
{

	if (abs_bottom >= abs_top ||
		abs_bottom	& 0x01	  || 
		abs_top		& 0x01	  || 
		abs(abs_top - abs_bottom) > CCU_MAX_PORT_RANGE)
	{
		LogCrit("Please check validity of the range bottom=[" << abs_bottom  << "] top=[" << abs_top << "] and that all numbers are even.");
		throw;
	}

	for (int i = abs_bottom; i <= abs_top; i++)
	{
		_portsList.push_back(i);
	}

}

PortManager::~PortManager(void)
{
	_portsList.clear();
}


int 
PortManager::GetNextPortFromPool()
{
	int port = _portsList.front();
	_portsList.erase(_portsList.begin());;
	return port;
}

void
PortManager::ReturnPortToPool(int port)
{
	_portsList.push_back(port);
}

}

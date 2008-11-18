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

#define CCU_MAX_PORT_RANGE 65000

PortManager::PortManager(int abs_top, int abs_bottom):
_absTop(abs_top),
_absBottom(abs_bottom),
_lastAllocatedPortSlot(CCU_UNDEFINED),
_portsMap(abs(abs_top - abs_bottom)), 
_counter(0)
{

	
	if (_absBottom >= _absTop ||
		_absBottom	& 0x01	  || 
		_absTop		& 0x01	  || 
		abs(abs_top - abs_bottom) > CCU_MAX_PORT_RANGE)
	{
		LogCrit("Please check validity of the range bottom=[" << _absBottom  << "] top=[" << _absTop << "] and that all numbers are even.");
		throw;
	}

	for (int i = _absBottom; i< abs_top; i++)
	{
		_portsMap[i - _absBottom] = CCU_PORT_AVAILABLE;
	}

	_numOfPortSlots =  _absTop - _absBottom;
}

PortManager::~PortManager(void)
{
	_portsMap.clear();
}

int
PortManager::NumOfPortSlots() const 
{ 
	return _numOfPortSlots; 
}

int 
PortManager::GetNextCandidate()
{
	//
	// find new port slot
	// 
	int curr_port_slot_candidate = CCU_UNDEFINED;
	if ( _lastAllocatedPortSlot == CCU_UNDEFINED)
	{
		curr_port_slot_candidate = _absBottom;
	} else 
	{
		curr_port_slot_candidate = (_lastAllocatedPortSlot + 2 <= _absTop) ?  
			_lastAllocatedPortSlot + 2 : _absBottom; 

	}

	if (_counter == NumOfPortSlots())
	{
		return CCU_UNDEFINED;
	}

	while (_counter < NumOfPortSlots())
	{

		if (_portsMap[curr_port_slot_candidate - _absBottom] != CCU_PORT_UNAVAILABLE)
		{
			_lastAllocatedPortSlot = curr_port_slot_candidate;
			return curr_port_slot_candidate;
		}

		curr_port_slot_candidate = (_lastAllocatedPortSlot + 2 <= _absTop) ?  
			_lastAllocatedPortSlot + 2 : _absBottom;

		_counter ++;
	}

	return CCU_UNDEFINED;


}

void
PortManager::MarkAvailable(int port)
{
	_portsMap[port - _absBottom]  = CCU_PORT_AVAILABLE;
}

void 
PortManager::MarkUnavailable(int port)
{
	_portsMap[port - _absBottom]  = CCU_PORT_UNAVAILABLE;
}

void 
PortManager::BeginSearch()
{
	_counter = 0;
}

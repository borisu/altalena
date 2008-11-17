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
#include "UIDOwner.h"



mutex UIDOwner::_mutex;

int UIDOwner::_pidIndex = 6000;


UIDOwner::UIDOwner(void)
{
	_lpid = GenerateNewUID();
}

UIDOwner::~UIDOwner(void)
{
}

int
UIDOwner::GenerateNewUID()
{
	mutex::scoped_lock scoped_lock(_mutex);

	int res = _pidIndex ++;

	return res;
}

int
UIDOwner::GetObjectUid() const
{
	return _lpid;
}


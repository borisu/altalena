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

#include "stdafx.h"
#include "ccu.h"


using namespace std;
using namespace csp;

wostream& operator << (wostream &ostream,  CnxInfo *ptr)
{
	if (ptr == NULL)
	{
		return ostream << "NULL";
	} 

	return ostream << ptr->ipporttows();
}

wostream& operator << (wostream &ostream, const CnxInfo &ptr)
{
	return ostream << ptr.ipporttows();
}

int operator == (const CnxInfo &right,const CnxInfo &left)
{
	return memcmp(&right.addr,&left.addr,sizeof(left.addr)) == 0;
}

Agent::Agent()
{

};

Agent::Agent(const Agent &other)
{
	name = other.name;

	media_address = other.media_address;

}



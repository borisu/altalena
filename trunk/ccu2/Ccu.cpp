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

wostream& operator << (wostream &ostream,  CcuMediaData *ptr)
{
	if (ptr == NULL)
	{
		return ostream << "NULL";
	} 

	char buffer[CCU_MAX_IP_LEN];
	buffer[0] = '\0';

	return ostream << ptr->ipporttoa(buffer, CCU_MAX_IP_LEN);
}

wostream& operator << (wostream &ostream, const CcuMediaData &ptr)
{
	in_addr temp;
	temp.S_un.S_addr = ptr.ip_addr;
	char * dest = ::inet_ntoa(temp); 

	return ostream << "ip_addr=[" << dest << "] port=[" << dec << ptr.port  << "]";
}

int operator == (const CcuMediaData &right,const CcuMediaData &left)
{
	return (
		(right.ip_addr	== left.ip_addr) && 
		(right.port		== left.port));
}

Agent::Agent()
{

};

Agent::Agent(const Agent &other)
{
	name = other.name;

	media_address = other.media_address;

}



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
#include "IwBase.h"
#include "Profiler.h"
#include "Configuration.h"


using namespace std;
using namespace csp;

namespace ivrworx
{

	critical_exception::critical_exception()
	{

	}

	critical_exception::critical_exception(const char *what):
	std::exception(what)
	{
		
	}

	

	IW_CORE_API ostream& operator << (ostream &ostream,  CnxInfo *ptr)
	{
		if (ptr == NULL)
		{
			return ostream << "NULL";
		} 

		return ostream << ptr->ipporttos();
	}

	IW_CORE_API ostream& operator << (ostream &ostream, const CnxInfo &ptr)
	{
		return ostream << ptr.ipporttos();
	}

	int operator == (const CnxInfo &right,const CnxInfo &left)
	{
		return memcmp(&right.addr,&left.addr,sizeof(left.addr)) == 0;
	}

	CnxInfo::CnxInfo(IN const char *hostport)
	{
		char *dup = ::strdup(hostport);
		char *host = ::strtok(dup,":");
		long port = ::atol(::strtok(dup,":"));

		init_from_hostname(host,port);

		::free (dup);


	}


	CnxInfo::CnxInfo(IN const string &s, IN int p_port)
	{
		init_from_hostname(s.c_str(),p_port);
	}

	CnxInfo::CnxInfo(IN const char *ip, IN int p_port)
	{
		init_from_hostname(ip,p_port);
	}

	CnxInfo::CnxInfo(IN in_addr p_in_addr, IN int p_port)
	{
		addr.sin_family = AF_INET;;
		addr.sin_addr = p_in_addr;
		addr.sin_port = ::htons(p_port);


		init_strings();
	}

	CnxInfo::CnxInfo()
	{
		addr.sin_addr.s_addr = INADDR_NONE;
		addr.sin_port = IW_UNDEFINED;
	}

	void CnxInfo::init_from_hostname(const char *host_name,int p_port)
	{
		
		unsigned long ia = inet_addr(host_name);

		addr.sin_addr.S_un.S_addr = ia;
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(p_port);
	
		init_strings();

	}

	BOOL
	CnxInfo::is_valid() const
	{
		return is_ip_valid() && is_port_valid();
	}

	BOOL 
	CnxInfo::is_ip_valid() const
	{
		return (addr.sin_addr.s_addr != INADDR_NONE );
	}

	BOOL 
	CnxInfo::is_port_valid() const
	{
		return (addr.sin_port != IW_UNDEFINED);
	}

	CnxInfo::CnxInfo(IN const CnxInfo &x)
	{
		addr = x.addr;
		saddr = x.saddr;

		init_strings();
	}

	int CnxInfo::port_ho() const
	{
		return ::ntohs(addr.sin_port);
	}

	int CnxInfo::port_no() const
	{
		return addr.sin_port;
	}

	in_addr CnxInfo::inaddr()
	{
		return addr.sin_addr;
	}

	long CnxInfo::iaddr_ho() const
	{
		return ::ntohl(addr.sin_addr.s_addr);
	}

	long CnxInfo::iaddr_no() const
	{
		return addr.sin_addr.s_addr;
	}

	sockaddr_in CnxInfo::sockaddr() const
	{
		return addr;
	}

	string CnxInfo::ipporttos() const
	{
		return saddrport;
	}

	string CnxInfo::iptos() const
	{
		return saddr;
	}

	string CnxInfo::porttos() const
	{
		return	sport;
	}

	const char *CnxInfo::ipporttoa() const
	{
		return saddrport.c_str();
	}

	const char *CnxInfo::iptoa() const
	{
		return saddr.c_str();
	}

	void CnxInfo::init_strings()
	{
		// convert port
		char buffer[10];
		buffer[0] = '\0';

		if ( _itoa_s(port_ho(),buffer,10,10) != 0)
		{
			sport = "INVALID";
		} 
		else
		{
			sport = buffer;
		}

		// convert address
		saddr = string(::inet_ntoa(addr.sin_addr));

		// build full address
		saddrport.append(saddr);
		saddrport.append(":");
		saddrport.append(sport);

	}

	in_addr convert_hname_to_addrin(const char *name)
	{
		hostent *phe = ::gethostbyname(name);
		if (phe == NULL)
		{
			DWORD last_error = ::GetLastError();
			cerr << "::gethostbyname returned error for host:" << name << ", le:" << last_error;
			throw configuration_exception();
		}


		// take only first result
		struct in_addr addr;
		addr.s_addr = *(u_long *) phe->h_addr;

		return addr;

	}

	const CnxInfo CnxInfo::UNKNOWN;


}


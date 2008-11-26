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

#include "CcuCommon.h"
#include "CcuMessage.h"



using namespace std;
using namespace boost;
using namespace boost::serialization;

enum CcuWellKnownProcesses
{
	RTP_RELAY_Q,			
	RTP_RELAY_IPC_Q,			
	IMS_Q,						
	VCS_Q,						
	COMMANDER_Q,					
	IPC_DISPATCHER_Q,			
	AIS_Q,						
	LAST_WELL_KNOWN_Q_MARKER    

};


#define IsWellKnownPid(x)  ((x) < LAST_WELL_KNOWN_Q_MARKER)


typedef long CcuConnectionId;

#define CCU_MAX_IP_LEN 64

namespace boost {
	namespace serialization {
		template<class Archive>
		void serialize(Archive & ar, sockaddr_in & g, const unsigned int version)
		{
			ar & make_nvp("sockadr_in",make_binary_object(&g,sizeof(sockaddr_in)));
		}
	}
}


class CcuMediaData
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(saddr);
		SERIALIZE_FIELD(addr);
	}

	sockaddr_in addr;

	string saddr;

	
public:


	CcuMediaData(wstring s, int p_port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(p_port);
		addr.sin_addr.s_addr = ::inet_addr(WStringToString(s).c_str());
		
		saddr = ipporttos();
	}

	CcuMediaData(string s, int p_port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(p_port);
		addr.sin_addr.s_addr = ::inet_addr(s.c_str());
		
		saddr = ipporttos();
	}

	CcuMediaData(char *ip, int p_port)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(p_port);
		addr.sin_addr.s_addr = ::inet_addr(ip);

		saddr = ipporttos();
	}

 	CcuMediaData(in_addr p_in_addr, int p_port)
 	{
 		addr.sin_family = AF_INET;;
		addr.sin_addr = p_in_addr;
		addr.sin_port = ::htons(p_port);
 		

		saddr = ipporttos();
 	}

	CcuMediaData()
	{
		addr.sin_addr.s_addr = INADDR_NONE;
		addr.sin_port = CCU_UNDEFINED;
	}

	bool is_valid()
	{
		return (addr.sin_addr.s_addr != INADDR_NONE );
	}

	CcuMediaData(const CcuMediaData &x)
	{
		addr = x.addr;
		saddr = x.saddr;
	}

	int port()
	{
		return ::ntohs(addr.sin_port);
	}

	sockaddr_in sockaddr()
	{
		return addr;
	}

	string ipporttos() const
	{
		
		if (!saddr.empty())
		{
			return saddr;
		}

		char buffer[CCU_MAX_IP_LEN];
		buffer[0] = '\0';
		return string(ipporttoa(buffer, CCU_MAX_IP_LEN));
	}

	string iptos() const
	{

		return string(iptoa());
	}

	string porttos() const
	{
		char buffer[10];
		buffer[0] = '\0';

		if ( _itoa_s(::ntohs(addr.sin_port),buffer,10,10) != 0)
		{
			return "NONE";
		}

		return string(buffer);
	}

	char *ipporttoa(char *buffer, int len) const
	{
		char* ipstr = ::inet_ntoa(addr.sin_addr); 

		sprintf_s(buffer,len,"%s:%d", ipstr, ::ntohs(addr.sin_port));
		return buffer;
	}

	char *iptoa() const
	{
		return ::inet_ntoa(addr.sin_addr); 
	}

	friend int operator == (const CcuMediaData &right,const CcuMediaData &left);

};
BOOST_CLASS_EXPORT(CcuMediaData);

typedef 
list<CcuMediaData> CcuMediaDataList;


wostream& operator << (wostream &ostream, const CcuMediaData *ptr);

wostream& operator << (wostream &ostream, const CcuMediaData &ptr);


//
// Business Objects
//
struct Agent
{	
	Agent();

	Agent(const Agent &other);

	wstring name;

	wstring media_address;
};

typedef 
list<Agent> AgentsList;


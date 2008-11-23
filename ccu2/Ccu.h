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

class CcuMediaData
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(ip_addr);
		SERIALIZE_FIELD(port);
	}

public:

	long ip_addr;

	int port;

	CcuMediaData(wstring s, int p_port)
	{
		ip_addr = ::inet_addr(WStringToString(s).c_str());
		port = p_port;

	}

	CcuMediaData(string s, int p_port)
	{
		ip_addr = ::inet_addr(s.c_str());
		port = p_port;

	}

	CcuMediaData(char *ip, int p_port)
	{
		ip_addr = ::inet_addr(ip);
		port = p_port;

	}

	CcuMediaData(DWORD p_ip_addr, int p_port)
	{
		ip_addr = p_ip_addr;
		port = p_port;

	}

	CcuMediaData()
	{
		ip_addr = INADDR_NONE;
		port = CCU_UNDEFINED;
	}

	bool IsValid()
	{
		return (ip_addr != INADDR_NONE && 
			port != CCU_UNDEFINED);
	}

	CcuMediaData(const CcuMediaData &x)
	{
		ip_addr = x.ip_addr;

		port = x.port;
	}

	string ipporttos() 
	{
		
		char buffer[CCU_MAX_IP_LEN];
		buffer[0] = '\0';
		return string(ipporttoa(buffer, CCU_MAX_IP_LEN));
	}

	string iptos() 
	{

		return string(iptoa());
	}

	string porttos()
	{
		char buffer[10];
		buffer[0] = '\0';
		if ( _itoa_s(port,buffer,10,10)!=0)
		{
			return "";
		}

		return string(buffer);
	}

	char *ipporttoa(char *buffer, int len) 
	{
		int curr_len = 0;

		in_addr temp;
		::ZeroMemory(&temp,sizeof(temp));
		temp.S_un.S_addr = ip_addr;
		char * ipstr = ::inet_ntoa(temp); 

		sprintf_s(buffer,len,"%s:%d",ipstr, port);
		return buffer;
	}

	char *iptoa() 
	{
		int curr_len = 0;

		in_addr temp;
		::ZeroMemory(&temp,sizeof(temp));
		temp.S_un.S_addr = ip_addr;
		char * ipstr = ::inet_ntoa(temp); 
		
		return ipstr;
	}

};
BOOST_CLASS_EXPORT(CcuMediaData);

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


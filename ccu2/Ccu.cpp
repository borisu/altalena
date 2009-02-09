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


IxCodec::IxCodec()
:name(L"INVALID"),
rate(IX_UNDEFINED),
mapping(IX_UNDEFINED)
{
	init_strings();
};

IxCodec::IxCodec(wstring param_name, int param_sampling_rate,int param_sdp_mapping)
:name(param_name),
rate(param_sampling_rate),
mapping(param_sdp_mapping)
{
	init_strings();
};

IxCodec::IxCodec(const IxCodec &codec)
{
	name = codec.name;
	rate = codec.rate;
	mapping = codec.mapping;

	init_strings();

};

int IxCodec::sampling_rate() const 
{ 
	return rate; 
}

string IxCodec::sampling_rate_tos() const 
{ 
	return rate_s; 
}

wstring IxCodec::sampling_rate_tows() const 
{ 
	return rate_ws; 
}

int IxCodec::sdp_mapping() const 
{ 
	return mapping; 
}

string IxCodec::sdp_mapping_tos() const 
{ 
	return mapping_s; 
}

string IxCodec::sdp_mapping_tows() const 
{ 
	return mapping_s; 
}

wstring IxCodec::sdp_name() const 
{ 
	return name; 
}

string IxCodec::sdp_name_tos() const 
{ 
	return name_s; 
}

string IxCodec::get_sdp_a() const
{
	return sdp_a;
}

void IxCodec::init_strings()
{
	WStringToUpper(name);

	// convert port
	char buffer[10];
	buffer[0] = '\0';

	// rate
	if ( _itoa_s(rate,buffer,10,10) != 0)
	{
		rate_s = "INVALID";
		rate_ws = L"INVALID";
	} 
	else
	{
		rate_s = string(buffer);
		rate_ws = StringToWString(rate_s);
	}

	// sdp mapping
	if ( _itoa_s(mapping,buffer,10,10) != 0)
	{
		mapping_s = "INVALID";
		mapping_ws = L"INVALID";
	} 
	else
	{
		mapping_s = string(buffer);
		mapping_ws = StringToWString(rate_s);
	}

	name_s  = WStringToString(name);
	sdp_a   += "a=rtpmap:" + sdp_mapping_tos() + " "  + sdp_name_tos() + "/" + sampling_rate_tos() + "\r\n";

}

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


CnxInfo::CnxInfo(wstring s, int p_port)
{
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(p_port);
	addr.sin_addr.s_addr = ::inet_addr(WStringToString(s).c_str());

	init_strings();
}

CnxInfo::CnxInfo(string s, int p_port)
{
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(p_port);
	addr.sin_addr.s_addr = ::inet_addr(s.c_str());

	init_strings();
}

CnxInfo::CnxInfo(char *ip, int p_port)
{
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(p_port);
	addr.sin_addr.s_addr = ::inet_addr(ip);

	init_strings();
}

CnxInfo::CnxInfo(in_addr p_in_addr, int p_port)
{
	addr.sin_family = AF_INET;;
	addr.sin_addr = p_in_addr;
	addr.sin_port = ::htons(p_port);


	init_strings();
}

CnxInfo::CnxInfo()
{
	addr.sin_addr.s_addr = INADDR_NONE;
	addr.sin_port = IX_UNDEFINED;
}

bool CnxInfo::is_ip_valid() const
{
	return (addr.sin_addr.s_addr != INADDR_NONE );
}

bool CnxInfo::is_port_valid() const
{
	return (addr.sin_port != IX_UNDEFINED);
}

CnxInfo::CnxInfo(const CnxInfo &x)
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

wstring CnxInfo::ipporttows() const
{
	return wsaddrport;
}

string CnxInfo::iptos() const
{
	return saddr;
}

string CnxInfo::porttos() const
{
	return	sport;
}

const char *CnxInfo::ipporttoa(char *buffer, int len) const
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

	wsport = StringToWString(sport);
	wsaddr = StringToWString(saddr);
	wsaddrport = StringToWString(saddrport);

}



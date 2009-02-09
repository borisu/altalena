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

struct IxCodec
{

	IxCodec();

	IxCodec(wstring param_name, int param_sampling_rate,int param_sdp_mapping);

	IxCodec(const IxCodec &codec);

	int sampling_rate() const;

	string sampling_rate_tos() const;

	wstring sampling_rate_tows() const;

	int sdp_mapping() const;

	string sdp_mapping_tos() const;

	string sdp_mapping_tows() const;

	wstring sdp_name() const;

	string sdp_name_tos() const;

	string get_sdp_a() const;

private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(rate);
		SERIALIZE_FIELD(mapping);
		SERIALIZE_FIELD(name);
	}

	void init_strings();

	int rate;

	wstring rate_ws;

	string rate_s;

	int mapping;

	wstring mapping_ws;

	string mapping_s;

	wstring name;

	string name_s;

	string sdp_a;

};
BOOST_CLASS_EXPORT(IxCodec);


typedef list<const IxCodec*> CodecsList;

class CnxInfo
{
private:

	BOOST_SERIALIZATION_REGION
	{
		SERIALIZE_FIELD(saddr);
		SERIALIZE_FIELD(addr);
	}

	sockaddr_in addr;

	wstring wsaddr;

	string saddr;

	wstring wsaddrport;

	string saddrport;

	string sport;

	wstring wsport;

	IxCodec codec;

public:

	CnxInfo(wstring s, int p_port);

	CnxInfo(string s, int p_port);

	CnxInfo(char *ip, int p_port);

	CnxInfo(in_addr p_in_addr, int p_port);

	CnxInfo(const CnxInfo &x);

	CnxInfo();

	bool is_ip_valid() const;

	bool is_port_valid() const;

	int port_ho() const;

	int port_no() const;

	in_addr inaddr();

	long iaddr_ho() const;

	long iaddr_no() const;

	sockaddr_in sockaddr() const;

	string ipporttos() const;

	wstring ipporttows() const;

	string iptos() const;

	string porttos() const;

	const char *ipporttoa(char *buffer, int len) const;

	const char *iptoa() const;

private:

	void init_strings();

	friend int operator == (const CnxInfo &right,const CnxInfo &left);

};
BOOST_CLASS_EXPORT(CnxInfo);

typedef 
list<CnxInfo> CcuMediaDataList;


wostream& operator << (wostream &ostream, const CnxInfo *ptr);

wostream& operator << (wostream &ostream, const CnxInfo &ptr);


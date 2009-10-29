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
	


	MediaFormat::MediaFormat()
		:name("INVALID"),
		rate(IW_UNDEFINED),
		mapping(IW_UNDEFINED),
		media_type(MediaType_UNKNOWN)
	{
		init_strings();
	};

	MediaFormat::MediaFormat(
		IN const string &param_name, 
		IN int param_sampling_rate,
		IN int param_sdp_mapping, 
		IN MediaType _media_type)
		:name(param_name),
		rate(param_sampling_rate),
		mapping(param_sdp_mapping),
		media_type(_media_type)
	{
		if (media_type == MediaFormat::MediaType_UNKNOWN)
		{
			media_type = GetMediaType(param_name);
		}
		init_strings();
	};

	MediaFormat::MediaFormat(IN const MediaFormat &other)
	{
		name = other.name;
		rate = other.rate;
		mapping = other.mapping;
		media_type = other.media_type;

		init_strings();

	};

	int MediaFormat::sampling_rate() const 
	{ 
		return rate; 
	}

	const string& MediaFormat::sampling_rate_tos() const 
	{ 
		return rate_s; 
	}

	int MediaFormat::sdp_mapping() const 
	{ 
		return mapping; 
	}

	const string& MediaFormat::sdp_mapping_tos() const 
	{ 
		return mapping_s; 
	}

	const string& MediaFormat::sdp_name_tos() const 
	{ 
		return name; 
	}


	const string& MediaFormat::get_sdp_a() const
	{
		return sdp_a;
	}

	MediaFormat::MediaType MediaFormat::get_media_type() const
	{
		return media_type;
	}

	void MediaFormat::init_strings()
	{

		// convert port
		char buffer[10];
		buffer[0] = '\0';

		// rate
		if ( _itoa_s(rate,buffer,10,10) != 0)
		{
			rate_s = "INVALID";
		} 
		else
		{
			rate_s = string(buffer);
		}

		// sdp mapping
		if ( _itoa_s(mapping,buffer,10,10) != 0)
		{
			mapping_s = "INVALID";
		} 
		else
		{
			mapping_s = string(buffer);
		}

		sdp_a   += "a=rtpmap:" + sdp_mapping_tos() + " "  + sdp_name_tos() + "/" + sampling_rate_tos() + "\r\n";

	}

	int
		MediaFormat::operator ==(const MediaFormat &other) const
	{
		return (
			(other.mapping == this->mapping) &&
			(other.rate == this->rate) &&
			(other.name == this->name));
	}

	const MediaFormat MediaFormat::PCMU("PCMU",8000,0, MediaType_SPEECH);

	const MediaFormat MediaFormat::PCMA("PCMA",8000,8, MediaType_SPEECH);

	const MediaFormat MediaFormat::SPEEX("SPEEX",8000,97, MediaType_SPEECH);

	const MediaFormat MediaFormat::DTMF_RFC2833("telephone-event", 8000, 101, MediaType_DTMF);

	MediaFormat::MediaType  MediaFormat::GetMediaType(const string &media_name)
	{
		if (media_name == "PCMA")
		{
			return MediaFormat::MediaType_SPEECH;
		}
		if (media_name == "PCMU")
		{
			return MediaFormat::MediaType_SPEECH;
		}
		if (media_name == "SPEEX")
		{
			return MediaFormat::MediaType_SPEECH;
		}
		if (media_name == "telephone-event")
		{
			return MediaFormat::MediaType_DTMF;
		}

		return MediaFormat::MediaType_UNKNOWN;

	}

	ostream& operator << (ostream &ostream,  const MediaFormat &ptr)
	{
		return ostream << "name:" << ptr.sdp_name_tos()<< " rate:" << ptr.sampling_rate() << " payload:" << ptr.sdp_mapping();
	}

	ostream& operator << (ostream &ostream,  CnxInfo *ptr)
	{
		if (ptr == NULL)
		{
			return ostream << "NULL";
		} 

		return ostream << ptr->ipporttos();
	}

	ostream& operator << (ostream &ostream, const CnxInfo &ptr)
	{
		return ostream << ptr.ipporttos();
	}

	int operator == (const CnxInfo &right,const CnxInfo &left)
	{
		return memcmp(&right.addr,&left.addr,sizeof(left.addr)) == 0;
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
		if (ia == INADDR_NONE || ia == INADDR_ANY)
		{
			throw std::exception("Invalid ip address supplied to CnxInfo ctor");
		}
		//else
		//{
		//	addr.sin_addr.S_un.S_addr = htonl( 0x7F000001 );
		//}

		
		addr.sin_addr.S_un.S_addr = ia;
		addr.sin_family = AF_INET;
		addr.sin_port = ::htons(p_port);
	
		init_strings();

	}

	bool CnxInfo::is_ip_valid() const
	{
		return (addr.sin_addr.s_addr != INADDR_NONE );
	}

	bool CnxInfo::is_port_valid() const
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


}


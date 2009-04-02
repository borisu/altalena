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

using namespace std;
using namespace boost;
using namespace boost::serialization;

namespace ivrworx
{
#define IW_UNDEFINED -1


	enum ApiErrorCode
	{
		API_SUCCESS = 0,
		API_FAILURE,
		API_SERVER_FAILURE,
		API_TIMEOUT,
		API_OUT_OF_BAND,
		API_UNKNOWN_PROC_DESTINATION,
		API_OPERATION_IN_PROGRESS 
	};

#define IW_SUCCESS(x)	((x) == API_SUCCESS)
#define IW_FAILURE(x)	((x) != API_SUCCESS)


	typedef int HandleId;

	typedef int ProcId;

	typedef LARGE_INTEGER TimeStamp;

	enum WellKnownProcAlias
	{
		IVR_Q,						
		IMS_Q,
		SIP_STACK_Q,
		LAST_WELL_KNOWN_Q_MARKER    
	};


#define IsWellKnownPid(x)  ((x) < LAST_WELL_KNOWN_Q_MARKER)

	typedef long ConnectionId;

	struct MediaFormat
	{
		enum MediaType
		{
			MediaType_UNKNOWN,
			MediaType_SPEECH,
			MediaType_DTMF
		};

		MediaFormat();

		MediaFormat(
			IN string param_name, 
			IN int param_sampling_rate,
			IN int param_sdp_mapping,
			IN MediaType media_type = MediaType_UNKNOWN);

		MediaFormat(const MediaFormat &codec);

		int sampling_rate() const;

		string sampling_rate_tos() const;

		int sdp_mapping() const;

		string sdp_mapping_tos() const;

		string sdp_name_tos() const;

		string get_sdp_a() const;

		MediaType get_media_type() const;

		static const MediaFormat PCMA;

		static const MediaFormat PCMU;

		static const MediaFormat DTMF;

		int operator == (const MediaFormat &other) const;

		static MediaType GetMediaType(string name);

	private:

		void init_strings();

		int rate;

		int mapping;

		string rate_s;

		string mapping_s;

		string name;

		string sdp_a;

		MediaType media_type;

	};
	
	typedef list<const MediaFormat*> MediaFormatsPtrList;

	typedef list<const MediaFormat> MediaFormatsList;

	typedef map<int,const MediaFormat> MediaFormatsMap;

	typedef pair<int,const MediaFormat> MediaFormatMapPair;

	ostream& operator << (ostream &ostream, const MediaFormat &ptr);

	class CnxInfo
	{
	private:

		sockaddr_in addr;

		string saddr;

		string saddrport;

		string sport;

		MediaFormat codec;

	public:

		CnxInfo(IN const string &host, IN int p_port);

		CnxInfo(IN const char *host, IN int p_port);

		CnxInfo(IN in_addr p_in_addr, IN int p_port);

		CnxInfo(IN const CnxInfo &x);

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

		string iptos() const;

		string porttos() const;

		const char *ipporttoa(char *buffer, int len) const;

		const char *iptoa() const;

	private:

		void init_from_hostname(const char *host_name,int port);

		void init_strings();

		friend int operator == (const CnxInfo &right,const CnxInfo &left);

	};

	typedef 
	list<CnxInfo> CnxInfosList;


	ostream& operator << (ostream &ostream, const CnxInfo *ptr);

	ostream& operator << (ostream &ostream, const CnxInfo &ptr);
}
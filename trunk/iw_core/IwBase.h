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


/**
@mainpage ivrworx

<B>Documenation is (as always) much work in progress. If you are missing some info, please write to <A HREF="borisusun@gmail.com">me</A> 
and I will add it ASAP.</B>

@section Introduction

ivrworx &reg; is an <A HREF="http://en.wikipedia.org/wiki/Interactive_voice_response">IVR</A>
application, where you can write your call flow scripts in <A HREF="http://www.lua.org/">Lua</A> programming language.
@section Documentation

- @ref scripts 
This section gives brief explanation on how to wirite ivrworx lua scripts.

- @ref configuration
This section explains how to configure the system

- @ref architecture
Take a look at this section if you are a developer or you would like to know more regarding the "under-the-hood" implementation of <b>ivrworx</b>.

@section Development Join The Development
The ivrworx is a project under development. We need working hands. Contact borisusun@gmail.com in order to join development team.


**/

/**

@defgroup architecture Architecture

@section Architecture

<b>ivrworx &reg;</b> is built around the idea that we may achieve simple synchronous telephony API script 
without sacrificing the efficiency. Asynchronous application are really the most efficient ones, and
usually telephony servers are built around this event-driven architecture. The state of the call is
remembered and retrieved at each event. It turns out that this style of programming is only good
for implementing a reasonably simple state machines. However, even for most simple applications like
IVR state machine asynchronous API turns out to be too complicated and cumbersome to use. 

The main problem when building synchronous API
for telephony application is thread management. You may not open too many threads on one hand and
you may not hang the thread waiting for response on another. Another problems with threads is that
highly parallelized applications are prone to race conditions and correct locking of a resources is a .

<b>ivrworx &reg;</b> solves the problem by utilizing fibers windows API in order to open. The fibers and its 
scheduling framework is encapsulated by CSP framework. For every incoming call new fiber is open
in IVR thread which load its own copy of lua virtual machine. Scripts are exposing simplistic
synchronous API to handle calls.

TBD

**/

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
		API_TIMEOUT
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
		WAITER_Q
	};

	/**

	Denotes SDP format (speech codec, dtmf encoding..)

	**/
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
			IN const string &param_name, 
			IN int param_sampling_rate,
			IN int param_sdp_mapping,
			IN MediaType media_type = MediaType_UNKNOWN);

		MediaFormat(IN const MediaFormat &codec);

		int sampling_rate() const;

		string sampling_rate_tos() const;

		int sdp_mapping() const;

		string sdp_mapping_tos() const;

		string sdp_name_tos() const;

		string get_sdp_a() const;

		MediaType get_media_type() const;

		static const MediaFormat PCMA;

		static const MediaFormat PCMU;

		static const MediaFormat DTMF_RFC2833;

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

	/**

	Encapsulates the ip and port information. If supplied host name tries to resolve it to ip addr structure.

	**/
	class CnxInfo
	{
	private:

		sockaddr_in addr;

		string saddr;

		string saddrport;

		string sport;

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
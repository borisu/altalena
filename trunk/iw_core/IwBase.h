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
#include "DllHelpers.h"

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
		API_SOCKET_INIT_FAILURE,
		API_TIMER_INIT_FAILURE,
		API_SERVER_FAILURE,
		API_TIMEOUT,
		API_WRONG_PARAMETER,
		API_WRONG_STATE,
		API_HANGUP,
		API_UNKNOWN_DESTINATION,
		API_FEATURE_DISABLED,
		API_UNKNOWN_RESPONSE,
		API_PENDING_OPERATION
	};

	#define IW_SUCCESS(x)	((x) == API_SUCCESS)
	#define IW_FAILURE(x)	((x) != API_SUCCESS)

	typedef int HandleId;

	typedef int ProcId;

	typedef LARGE_INTEGER TimeStamp;

	class IW_CORE_API critical_exception: public exception 
	{
	public:
		critical_exception();
		critical_exception(const char *);
	};

	

	IW_CORE_API in_addr convert_hname_to_addrin(const char *name);

	/**

	Encapsulates the ip and port information. If supplied host name tries to resolve it to ip addr structure.

	**/
	class IW_CORE_API CnxInfo
	{
	private:

		sockaddr_in addr;

		string saddr;

		string saddrport;

		string sport;

	public:

		CnxInfo(IN const char *hostport);

		CnxInfo(IN const string &host, IN int p_port);

		CnxInfo(IN const char *host, IN int p_port);

		CnxInfo(IN in_addr p_in_addr, IN int p_port);

		CnxInfo(IN const CnxInfo &x);

		CnxInfo();

		BOOL is_valid() const;

		BOOL is_ip_valid() const;

		BOOL is_port_valid() const;

		int port_ho() const;

		int port_no() const;

		in_addr inaddr();

		long iaddr_ho() const;

		long iaddr_no() const;

		sockaddr_in sockaddr() const;

		string ipporttos() const;

		string iptos() const;

		string porttos() const;

		const char *ipporttoa() const;

		const char *iptoa() const;

		static const CnxInfo UNKNOWN;

	private:

		void init_from_hostname(const char *host_name,int port);

		void init_strings();

		friend int operator == (const CnxInfo &right,const CnxInfo &left);

	};

	

	typedef 
	list<CnxInfo> CnxInfosList;

	typedef
	map<string, string> MapOfAny;


	IW_CORE_API ostream& operator << (ostream &ostream, const CnxInfo *ptr);

	IW_CORE_API ostream& operator << (ostream &ostream, const CnxInfo &ptr);
}
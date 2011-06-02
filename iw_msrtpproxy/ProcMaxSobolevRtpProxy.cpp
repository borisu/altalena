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

#include "StdAfx.h"
#include "ProcMaxSobolevRtpProxy.h"

namespace ivrworx
{
	ProcMaxSobolevRtpProxy::ProcMaxSobolevRtpProxy(ConfigurationPtr conf,LpHandlePair pair):
	LightweightProcess(pair,"ProcMaxSobolevRtpProxy"),
	_conf(conf),
	_s(NULL)
	{
		FUNCTRACKER;

		ServiceId(_conf->GetString("msrtpproxy/uri"));
	}

	void 
	ProcMaxSobolevRtpProxy::real_run()
	{
		FUNCTRACKER;

		if (IW_FAILURE(InitSockets()))
			throw critical_exception("Cannot init neworking layer:");

		I_AM_READY;

		WSACleanup();
	}

	ProcMaxSobolevRtpProxy::~ProcMaxSobolevRtpProxy(void)
	{
		FUNCTRACKER;
	}

	ApiErrorCode
	ProcMaxSobolevRtpProxy::InitSockets()
	{
		FUNCTRACKER;

		WSADATA wsadata;
		int error = ::WSAStartup(0x0202, &wsadata);

		//Did something happen?
		if (error)
		{
			LogSysError("::WSAStartup");
			return API_FAILURE;
		}

		//Did we get the right Winsock version?
		if (wsadata.wVersion != 0x0202)
		{
			LogWarn("Incorrect socket version:" <<wsadata.wVersion);
			WSACleanup(); //Clean up Winsock
			return API_FAILURE;
		}

		_s = ::socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP); //Create socket
		if (_s == INVALID_SOCKET)
		{
			LogSysError("::socket");
			WSACleanup();
			return API_FAILURE; //Couldn't create the socket
		}  

		int port = _conf->GetInt("msrtpproxy/control_port");
		string host = _conf->GetString("msrtpproxy/control_host");

		struct hostent *remoteHost = NULL;
		remoteHost = ::gethostbyname(host.c_str());
		if (remoteHost == NULL)
		{
			LogSysError("::gethostbyname");
			WSACleanup();
			return API_FAILURE; //Couldn't create the socket
		}

		SOCKADDR_IN target; //Socket address information
		target.sin_family = AF_INET; // address family Internet
		target.sin_port = htons (port); //Port to connect on
		target.sin_addr.s_addr = *(u_long *) remoteHost->h_addr; //Target IP

		//Try connecting...
		if (::connect(_s, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
		{
			LogSysError("::connect");
			WSACleanup();
			return API_FAILURE;  //Couldn't connect

		}

		return API_SUCCESS;
		
	}

}


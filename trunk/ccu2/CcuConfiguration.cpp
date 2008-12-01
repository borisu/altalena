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
#include "CcuConfiguration.h"
#include "CcuLogger.h"

using namespace boost;


CcuConfiguration::CcuConfiguration(void)
{
}

CcuConfiguration::~CcuConfiguration(void)
{
	_agentsList.clear();
	_agentsMap.clear();
}

void
CcuConfiguration::Agents(AgentsList &agents_list)
{
	mutex::scoped_lock lock(_mutex);

	if (!agents_list.empty())
	{
		LogWarn("Agents list passed as a parameter is not empty.");
	}
	

	agents_list.assign(_agentsList.begin(), _agentsList.end());

}

void
CcuConfiguration::AddAgent(const Agent &agent)
{
	mutex::scoped_lock lock(_mutex);

	if (agent.name == L"")
	{
		LogWarn("Not valid agent passed as a parameter.");
		return;
	}

	_agentsList.push_back(agent);
	_agentsMap[agent.name] = agent;

}

CnxInfo
CcuConfiguration::DefaultCnxInfo()
{
	mutex::scoped_lock lock(_mutex);

	return _defaultIp;

}

CnxInfo
CcuConfiguration::VcsCnxInfo()
{
	mutex::scoped_lock lock(_mutex);

	return _vcsMediaData;

}

int
CcuConfiguration::RtpRelayTopPort()
{
	mutex::scoped_lock lock(_mutex);

	return _rtpRelayTopPort;

}

int
CcuConfiguration::RtpRelayBottomPort()
{
	mutex::scoped_lock lock(_mutex);

	return _rtpRelayBottomPort;

}

CnxInfo
CcuConfiguration::RtpRelayIp()
{
	mutex::scoped_lock lock(_mutex);

	return _rtpRelayIp;

}


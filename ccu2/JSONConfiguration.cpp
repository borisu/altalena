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
#include "JSONConfiguration.h"
#include "CcuLogger.h"

using namespace json_spirit;

bool same_name( const wPair& pair, const wstring& name )
{
	return pair.name_ == name;
}

const wValue& find_value( const wObject& obj, const wstring& name )
{
	wObject::const_iterator i = find_if( obj.begin(), obj.end(), bind( same_name, _1, ref( name ) ) );

	if( i == obj.end() ) return wValue::null;

	return i->value_;
}

const wArray& find_array( const wObject& obj, const wstring& name )
{
	return find_value( obj, name ).get_array(); 
}

int find_int( const wObject& obj, const wstring& name )
{
	return find_value( obj, name ).get_int();
}

wstring find_str( const wObject& obj, const wstring& name )
{
	return find_value( obj, name ).get_str();
}

void read_agent( const wObject& obj, Agent &agent)
{
	agent.media_address = find_str( obj, L"media_address" );
	agent.name          = find_str( obj, L"name" );        
}


JSONConfiguration::JSONConfiguration(void)
{
}

JSONConfiguration::~JSONConfiguration(void)
{
}

CcuApiErrorCode
JSONConfiguration::InitFromFile(const wstring &filename)
{
	wifstream is(WStringToString(filename).c_str());

	if (read(is, _value) == false)
	{
		LogCrit(L"Error reading JSON configuration file=[" << filename << L"]");
		throw;
	}


	CcuApiErrorCode res = InitDb();
	return res;

}

CcuApiErrorCode
JSONConfiguration::InitFromString(const wstring &is)
{
	
	if (read(is, _value) == false)
	{
		return CCU_API_FAILURE;
	}

	CcuApiErrorCode res = InitDb();
	return res;

}



CcuApiErrorCode
JSONConfiguration::InitDb()
{

	wObject root_obj(_value.get_obj());

	//
	// agents
	//
	const wArray& addr_array( find_array(root_obj, L"agents" ) );

	for( unsigned int i = 0; i < addr_array.size(); ++i )
	{
		Agent temp_agent;
		read_agent( addr_array[i].get_obj(),temp_agent );

		AddAgent(temp_agent);
	}

	//
	// default ip
	//
	const wstring default_ip_str = find_str(root_obj, L"default_ip" );

	_defaultIp = CcuMediaData(default_ip_str,5060);


	//
	// vcs ip
	//

	const wstring vcs_ip_str = find_str(root_obj, L"vcs_sip_ip" );
	const int vcs_ip_int = find_int(root_obj, L"vcs_sip_port" );

	_vcsMediaData = CcuMediaData(vcs_ip_str,vcs_ip_int);


	return CCU_API_SUCCESS;

}

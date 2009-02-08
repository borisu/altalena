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

namespace ivrworx
{

	static bool 
	same_name( const wPair& pair, const wstring& name )
	{
		return pair.name_ == name;
	}

	static const wValue& 
	find_value( const wObject& obj, const wstring& name )
	{
		wObject::const_iterator i = find_if( obj.begin(), obj.end(), bind( same_name, _1, ref( name ) ) );

		if( i == obj.end() ) return wValue::null;

		return i->value_;
	}

	static const wArray& 
	find_array( const wObject& obj, const wstring& name )
	{
		return find_value( obj, name ).get_array(); 
	}

	static int 
	find_int( const wObject& obj, const wstring& name )
	{
		return find_value( obj, name ).get_int();
	}

	static wstring 
	find_str( const wObject& obj, const wstring& name )
	{
		return find_value( obj, name ).get_str();
	}

	IxCodec *
	read_codec( const wObject& obj)
	{
		return 
			new IxCodec(
				find_str( obj, L"name" ),
				find_int( obj, L"sampling_rate" ),
				find_int( obj, L"sdp_mapping" ));
	}


	JSONConfiguration::JSONConfiguration(void)
	{
	}

	JSONConfiguration::~JSONConfiguration(void)
	{
	}

	IxApiErrorCode
	JSONConfiguration::InitFromFile(const wstring &filename)
	{
		wifstream is(WStringToString(filename).c_str());

		if (read(is, _value) == false)
		{
			LogCrit(L"Error reading JSON configuration file=[" << filename << L"]");
			throw;
		}


		IxApiErrorCode res = InitDb();
		return res;

	}

	IxApiErrorCode
	JSONConfiguration::InitFromString(const wstring &is)
	{

		if (read(is, _value) == false)
		{
			return CCU_API_FAILURE;
		}

		IxApiErrorCode res = InitDb();
		return res;

	}



	IxApiErrorCode
	JSONConfiguration::InitDb()
	{

		wObject root_obj(_value.get_obj());

		//
		// default ip
		//
		const wstring default_ip_str = find_str(root_obj, L"default_ip" );

		_defaultIp = CnxInfo(default_ip_str,5060);


		//
		// vcs 
		//
		const wstring vcs_ip_str = find_str(root_obj, L"vcs_sip_ip" );
		const int vcs_ip_int = find_int(root_obj, L"vcs_sip_port" );

		_vcsMediaData = CnxInfo(vcs_ip_str,vcs_ip_int);

		//
		// ims
		//
		const wstring ims_ip_str = find_str(root_obj, L"ims_ip" );
		const int ims_ip_int = find_int(root_obj, L"ims_port" );

		_imsCnxInfo = CnxInfo(ims_ip_str,ims_ip_int);


		// rtp 

		const wstring rtp_relay_ip_str = find_str(root_obj, L"rtp_relay_ip" );

		const int rtp_relay_top_port_int = find_int(root_obj, L"rtp_relay_top_port" );
		const int rtp_relay_bottom_port_int = find_int(root_obj, L"rtp_relay_bottom_port" );

		_rtpRelayIp = CnxInfo(vcs_ip_str,IX_UNDEFINED);

		_rtpRelayTopPort = rtp_relay_top_port_int;
		_rtpRelayBottomPort = rtp_relay_bottom_port_int;

		// configuration file
		_scriptFile = find_str(root_obj, L"script_file");

		// codecs

		const wArray &codecs_array = find_array(root_obj, L"codecs");

		wArray::const_iterator iter = codecs_array.begin();
		while(iter != codecs_array.end())
		{
			IxCodec *codec;
			codec = read_codec(iter->get_obj());
			_codecsList.push_front(codec);
			iter++;
		}

		// from 
		_from		 = find_str(root_obj, L"from_id");
		_fromDisplay = find_str(root_obj, L"from_display_name");

		return CCU_API_SUCCESS;

	}

}

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
#include "Logger.h"

using namespace json_spirit;

namespace ivrworx
{

	static bool 
	same_name(const Pair& pair, const string& name )
	{
		return pair.name_ == name;
	}

	static const Value& 
	find_value( const Object& obj, const string& name )
	{
		Object::const_iterator i = find_if( obj.begin(), obj.end(), bind( same_name, _1, ref( name ) ) );

		if( i == obj.end() ) return Value::null;

		return i->value_;
	}

	static const Array& 
	find_array( const Object& obj, const string& name )
	{
		return find_value( obj, name ).get_array(); 
	}

	static int 
	find_int( const Object& obj, const string& name )
	{
		return find_value( obj, name ).get_int();
	}

	string
	get_env_variable_as_str(const char *var_name)
	{
        
		char* var_value = NULL;
		size_t requiredSize;

		::getenv_s( &requiredSize, NULL, 0, var_name);

		var_value = (char*)::malloc(requiredSize * sizeof(char));
		if (!var_value)
		{
			std::cerr << "Failed to allocate memory!";
			throw;
		}

		// Get the value of the LIB environment variable.
		::getenv_s( &requiredSize, var_value, requiredSize, var_name);

		string res(var_value);

		::free(var_value);

		return res;


	}

	static string 
	find_str( const Object& obj, const string& name )
	{
		
		string value_str = find_value( obj, name ).get_str();
		
		if (value_str.length() > 0 && 
			 *value_str.begin() == '$')
		{
			return get_env_variable_as_str(value_str.c_str() + 1);
		}

		return find_value( obj, name ).get_str();
	}

	MediaFormat *
	read_codec( const Object& obj)
	{
		string media_format_name = find_str(obj, "name" );

		MediaFormat::MediaType media_type = MediaFormat::GetMediaType(media_format_name);
		if ( media_type == MediaFormat::MediaType_UNKNOWN)
		{
			LogCrit("Unknown media format " << media_format_name);
			throw;
		}

		return new MediaFormat(
				media_format_name,
				find_int( obj, "sampling_rate" ),
				find_int( obj, "sdp_mapping" ),
				media_type);
	}


	JSONConfiguration::JSONConfiguration(void)
	{
	}

	JSONConfiguration::~JSONConfiguration(void)
	{
	}

	ApiErrorCode
	JSONConfiguration::InitFromFile(const string &filename)
	{
		ifstream is(filename.c_str());

		if (read(is, _value) == false)
		{
			LogCrit("Error reading JSON configuration file [" << filename << "].");
			throw;
		}


		ApiErrorCode res = InitDb();
		return res;
	}

	ApiErrorCode
	JSONConfiguration::InitFromString(const string &is)
	{

		if (read(is, _value) == false)
		{
			return API_FAILURE;
		}

		ApiErrorCode res = InitDb();
		return res;

	}



	ApiErrorCode
	JSONConfiguration::InitDb()
	{

		Object root_obj(_value.get_obj());

		//
		// debug level
		//
		_debugLevel = find_str(root_obj, "debug_level");

		//
		// syslog
		//
		_sysloghost = find_str(root_obj, "syslogd_host");
		_syslogport = find_int(root_obj, "syslogd_port");


		//
		// ivr 
		//
		const string ivr_host_str = find_str(root_obj, "ivr_sip_ip" );
		const int ivr_ip_int	= find_int(root_obj, "ivr_sip_port" );

		_ivrCnxInfo = CnxInfo(ivr_host_str,ivr_ip_int);

		//
		// ims
		//
		const string ims_host_str = find_str(root_obj, "ims_ip" );
		
		_imsCnxInfo = CnxInfo(ims_host_str,0);
		_imsTopPort		= find_int(root_obj, "ims_top_port" );
		_imsBottomPort	= find_int(root_obj, "ims_bottom_port" );



		// configuration file
		_scriptFile = find_str(root_obj, "script_file");

		// codecs
		const Array &codecs_array = find_array(root_obj, "codecs");

		Array::const_iterator iter = codecs_array.begin();
		while(iter != codecs_array.end())
		{
			MediaFormat *codec;
			codec = read_codec(iter->get_obj());
			_codecsList.push_front(codec);
			iter++;
		}

		// from 
		_from		 = find_str(root_obj, "from_id");
		_fromDisplay = find_str(root_obj, "from_display_name");

		// basic sound path
		_soundsPath	= find_str(root_obj, "sounds_dir");
		

		return API_SUCCESS;

	}

}

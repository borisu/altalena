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

	static const Value&
	query_value_with_path(const Object& parent_object, const string& name )
	{
		size_t found = name.find_first_of("/");
		if (found == string::npos)
		{
			return find_value(parent_object,name);
		}

		string next_parent_key =  
			name.substr (0,found);

		string remainder = 
			name.substr(found + 1, name.length());

		const Value& val = 
			find_value(parent_object,next_parent_key);

		if (val.type() != obj_type)
		{
			throw configuration_exception("path not found, next_parent_key is not an object");
		}

		return query_value_with_path(val.get_obj(), remainder);

	}

	static const Array& 
	find_array( const Object& obj, const string& name )
	{
		const Value & val = query_value_with_path(obj, name );
		if ( val.type() != array_type)
		{
			throw configuration_exception (name + " does not exist or not array type");
		};

		return val.get_array(); 
	}

	static int 
	find_int( const Object& obj, const string& name )
	{
		const Value & val = query_value_with_path(obj, name );
		if ( val.type() != int_type)
		{
			throw configuration_exception (name + " does not exist or not int type");
		};
		return val.get_int();
	}

	static bool
	find_bool( const Object& obj, const string& name)
	{
		const Value & val = query_value_with_path(obj, name );
		if ( val.type() != bool_type)
		{
			throw configuration_exception (name + " does not exist or not bool type");
		};
		return val.get_bool();
	}

	string
	get_env_variable_as_str(const char *var_name)
	{
        
		char* var_value = NULL;
		size_t requiredSize;

		::getenv_s( &requiredSize, NULL, 0, var_name);
		var_value = (char*)::malloc(requiredSize * sizeof(char));

		// Get the value of the LIB environment variable.
		::getenv_s( &requiredSize, var_value, requiredSize, var_name);

		string res(var_value);

		::free(var_value);

		return res;


	}

	static string 
	find_str( const Object& obj, const string& name )
	{
		
		
		const Value & val = query_value_with_path(obj, name );
		if ( val.type() != str_type)
		{
			throw configuration_exception (name + " does not exist or not string type");
		};

		string value_str = val.get_str();
		if (value_str.length() > 0 && 
			 *value_str.begin() == '$')
		{
			return get_env_variable_as_str(value_str.c_str() + 1);
		}

		return value_str;
	}

	JSONConfiguration::JSONConfiguration(void)
	{
		
	}

	JSONConfiguration::~JSONConfiguration(void)
	{
	}


	int
	JSONConfiguration::GetInt(IN const string &name)
	{
		return find_int(_rootValue.get_obj(),name);
	}

	string
	JSONConfiguration::GetString(IN const string &name)
	{

		return find_str(_rootValue.get_obj(),name);
				
	}

	BOOL
	JSONConfiguration::GetBool(IN const string &name)
	{

		return find_bool(_rootValue.get_obj(),name);

	}

	BOOL 
	JSONConfiguration::HasOption(const string &name)
	{
		try
		{
			const Value& val = 
				query_value_with_path(_rootValue.get_obj(),name);
			return (val.type() != null_type);
		}
		catch (configuration_exception e)
		{
			return FALSE;
		}
		
	}

	void 
	JSONConfiguration::GetArray(IN const string &name, OUT list<any> &out_list)
	{
		const Array &val_array = 
			find_array(_rootValue.get_obj(),name);

		out_list.clear();

		for (Array::const_iterator iter = val_array.begin();
			 iter != val_array.end();
		     iter++)
		{
			Value_type vt = iter->type();
			switch (vt)
			{
			case str_type: 
				{
					out_list.push_back(iter->get_str());
					break;
				}
			case bool_type: 
				{
					out_list.push_back(iter->get_bool());
					break;
				}
			case int_type: 
				{
					out_list.push_back(iter->get_int());
					break;
				}
			case real_type: 
				{
					out_list.push_back(iter->get_real());
					break;
				}
			case obj_type:
			case null_type:
			case array_type:
			default:
				{
					throw new configuration_exception("not supported JSON type. (TBD)");

				}
			} // switch
		}
	}



	ApiErrorCode
	JSONConfiguration::InitFromFile(IN const string &filename)
	{
		
		cout << "Dumping Configuration...\n";
		ifstream is1(filename.c_str());
		if (is1.is_open())
		{
			string line;
			while ( is1.good() )
			{
				getline (is1,line);
				cout << line << endl;

			}
			is1.close();
		} else
		{
			cerr << "Error reading json configuration file '" << filename << "'. Check that file exists ans accessible." << endl;
			return API_FAILURE;
		}

		ifstream  is(filename.c_str());
		if (read(is, _rootValue) == false)
		{
			cerr << "Error reading json configuration file '" << filename << "'. Check that file exists, accessible and JSON valid." << endl;
			return API_FAILURE;
		}

		return API_SUCCESS;
	}



}

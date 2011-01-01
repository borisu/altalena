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
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

using namespace std;
using namespace boost;




namespace ivrworx
{
	class configuration_exception: 	public exception 
	{
	public:
		configuration_exception();
		configuration_exception(const char *);
		configuration_exception(const string &);
	};

	typedef
	list<boost::any> ListOfAny;

	// Configuration class follows basic JSON semantics
	// though it may be implemented as XML or Db or any
	// other mechanism
	class Configuration
		:public boost::noncopyable
	{
	public:

		Configuration(void);

		virtual ~Configuration(void);

		virtual int GetInt(IN const string &name) = 0;

		virtual string GetString(IN const string &name) = 0;

		virtual BOOL GetBool(IN const string &name) = 0;

		virtual BOOL HasOption(IN const string &name) = 0;

		virtual void GetArray(IN const string &key, OUT ListOfAny &out_list) = 0;
		
	};

	typedef 
	shared_ptr<Configuration> ConfigurationPtr;

}


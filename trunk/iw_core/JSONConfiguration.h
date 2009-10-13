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

#include "Configuration.h"

using namespace std;
using namespace json_spirit;

namespace ivrworx
{

	/**
	*	@defgroup jsonconfiguration Json Configuration
	*
	*	Reads the @ref configuration configuration from <a href="http://en.wikipedia.org/wiki/json">Json</a> file.
	*	
	*/
	class JSONConfiguration :
		public Configuration
	{
	public:

		JSONConfiguration();

		virtual ~JSONConfiguration(void);

		/**
		*	Reads Json @ref configuration from file indicated in parameter.
		*
		*	@param_in path Name to json configuration file.
		*
		*	@return Result of operation.
		**/
		ApiErrorCode InitFromFile(IN const string &filename);

		/**
		*	Reads Json @ref configuration from string buffer.
		*
		*	@param_in path Buffer containing the valid json input.
		*
		*	@return Result of operation.
		**/
		ApiErrorCode InitFromString(IN const string &is);

		virtual int GetInt(const string &name);

		virtual string GetString(const string &name);

	private:

		ApiErrorCode InitDb();

		Value _value;
	};

}


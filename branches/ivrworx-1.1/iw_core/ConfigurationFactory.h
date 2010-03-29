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

namespace ivrworx
{

	typedef 
	shared_ptr<Configuration> ConfigurationPtr;

	/**
	*
	*	Configuration Factory.
	*
	*	This class is used to hide @ref configuration data source dependencies from including files.
	*	Every new configuration should supply factory class so using files will not have to include 
	*	dependent headers.
	*	
	*
	**/
	class ConfigurationFactory
	{
	public:

		ConfigurationFactory(void);

		virtual ~ConfigurationFactory(void);

		static ConfigurationPtr CreateJsonConfiguration(const string &filename, ApiErrorCode &err_code);

	};

}
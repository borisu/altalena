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
#include "Configuration.h"
#include "Logger.h"

using namespace boost;

namespace ivrworx
{


	Configuration::Configuration(void)
	{

	}

	Configuration::~Configuration(void)
	{
		while (!_mediaFormatPtrsList.empty())
		{
			const MediaFormat *ptr = _mediaFormatPtrsList.front();
			_mediaFormatPtrsList.pop_front();
			delete const_cast<MediaFormat *> (ptr);
		}
	}

	CnxInfo
	Configuration::DefaultCnxInfo()
	{
		mutex::scoped_lock lock(_mutex);

		return _defaultIp;

	}

	CnxInfo
	Configuration::IvrCnxInfo()
	{
		mutex::scoped_lock lock(_mutex);

		return _ivrCnxInfo;

	}

	CnxInfo
	Configuration::ImsCnxInfo()
	{
		mutex::scoped_lock lock(_mutex);

		return _imsCnxInfo;

	}

	string
	Configuration::ScriptFile()
	{
		mutex::scoped_lock lock(_mutex);

		return  _scriptFile;
	}

	int 
	Configuration::ImsTopPort()
	{
		mutex::scoped_lock lock(_mutex);

		return  _imsTopPort;
	}

	int 
	Configuration::ImsBottomPort()
	{
		mutex::scoped_lock lock(_mutex);

		return  _imsBottomPort;
	}


	void
	Configuration::AddMediaFormat(const MediaFormat& codec)
	{
		mutex::scoped_lock lock(_mutex);

		_mediaFormatPtrsList.push_front(new MediaFormat(codec));

	}


	void
	Configuration::AddMediaFormat(const MediaFormat* codec)
	{
		mutex::scoped_lock lock(_mutex);

		_mediaFormatPtrsList.push_front(codec);

	}


	const MediaFormatsPtrList& 
	Configuration::MediaFormats()
	{
		mutex::scoped_lock lock(_mutex);

		return _mediaFormatPtrsList;

	}

	string 
	Configuration::From()
	{
		mutex::scoped_lock lock(_mutex);

		return _from;

	}

	string 
	Configuration::SoundsPath()
	{
		mutex::scoped_lock lock(_mutex);

		return _soundsPath;

	}

	string 
	Configuration::FromDisplay()
	{
		mutex::scoped_lock lock(_mutex);

		return _fromDisplay;

	}

	string 
	Configuration::SyslogHost()
	{
		mutex::scoped_lock lock(_mutex);

		return _sysloghost;

	}

	int
	Configuration::SyslogPort()
	{
		mutex::scoped_lock lock(_mutex);

		return _syslogport;

	}

	string
	Configuration::DebugLevel()
	{
		mutex::scoped_lock lock(_mutex);

		return _debugLevel;

	}

	string
	Configuration::DebugOutputs()
	{
		mutex::scoped_lock lock(_mutex);

		return _debugOutputs;

	}

	string
	Configuration::ResipLog()
	{
		mutex::scoped_lock lock(_mutex);

		return _resipLog;

	}


}


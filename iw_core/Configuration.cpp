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
		while (_codecsList.empty() != 0)
		{
			const MediaFormat *ptr = *_codecsList.end();
			_codecsList.pop_back();
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


	void
	Configuration::AddCodec(const MediaFormat& codec)
	{
		mutex::scoped_lock lock(_mutex);

		_codecsList.push_front(new MediaFormat(codec));

	}


	void
	Configuration::AddCodec(const MediaFormat* codec)
	{
		mutex::scoped_lock lock(_mutex);

		_codecsList.push_front(codec);

	}


	const CodecsPtrList& 
	Configuration::CodecList()
	{
		mutex::scoped_lock lock(_mutex);

		return _codecsList;

	}

	string 
	Configuration::From()
	{
		mutex::scoped_lock lock(_mutex);

		return _from;

	}

	string 
	Configuration::FromDisplay()
	{
		mutex::scoped_lock lock(_mutex);

		return _fromDisplay;

	}

}


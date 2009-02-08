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

namespace ivrworx
{


	CcuConfiguration::CcuConfiguration(void)
	{
	}

	CcuConfiguration::~CcuConfiguration(void)
	{
		while (_codecsList.empty() != 0)
		{
			const IxCodec *ptr = *_codecsList.end();
			_codecsList.pop_back();
			delete const_cast<IxCodec *> (ptr);
		}
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

	CnxInfo
	CcuConfiguration::ImsCnxInfo()
	{
		mutex::scoped_lock lock(_mutex);

		return _imsCnxInfo;

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

	wstring
	CcuConfiguration::ScriptFile()
	{
		mutex::scoped_lock lock(_mutex);

		return  _scriptFile;
	}


	void
	CcuConfiguration::AddCodec(const IxCodec& codec)
	{
		mutex::scoped_lock lock(_mutex);

		_codecsList.push_front(new IxCodec(codec));

	}


	void
	CcuConfiguration::AddCodec(const IxCodec* codec)
	{
		mutex::scoped_lock lock(_mutex);

		_codecsList.push_front(codec);

	}


	const CodecsList& 
	CcuConfiguration::CodecList()
	{
		mutex::scoped_lock lock(_mutex);

		return _codecsList;

	}

	wstring 
	CcuConfiguration::From()
	{
		mutex::scoped_lock lock(_mutex);

		return _from;

	}

	wstring 
	CcuConfiguration::FromDisplay()
	{
		mutex::scoped_lock lock(_mutex);

		return _fromDisplay;

	}

}


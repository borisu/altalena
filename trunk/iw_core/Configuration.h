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
#include "IwBase.h"

using namespace std;

namespace ivrworx
{

	class Configuration
	{
	public:

		Configuration(void);

		virtual ~Configuration(void);

		virtual CnxInfo DefaultCnxInfo();

		virtual CnxInfo IvrCnxInfo();

		virtual CnxInfo ImsCnxInfo();

		virtual CnxInfo RtpRelayIp();

		virtual string ScriptFile();

		virtual void AddCodec(const MediaFormat& codec);

		virtual void AddCodec(const MediaFormat* codec);

		virtual const CodecsPtrList& CodecList();

		virtual string From();

		virtual string FromDisplay();

	protected:

		mutex _mutex;

		CnxInfo _defaultIp;

		CnxInfo _ivrCnxInfo;

		CnxInfo _imsCnxInfo;

		string _scriptFile;

		CodecsPtrList _codecsList;

		string _from;

		string _fromDisplay;

	};

}


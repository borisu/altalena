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
#include "Ccu.h"

using namespace std;

namespace ivrworx
{

	class CcuConfiguration
	{
	public:

		CcuConfiguration(void);

		virtual ~CcuConfiguration(void);

		virtual int RtpRelayBottomPort();

		virtual int RtpRelayTopPort();

		virtual CnxInfo DefaultCnxInfo();

		virtual CnxInfo VcsCnxInfo();

		virtual CnxInfo RtpRelayIp();

		virtual wstring ScriptFile();

		virtual void AddCodec(const IxCodec& codec);

		virtual void AddCodec(const IxCodec* codec);

		virtual const CodecsList& CodecList();

		virtual wstring From();

		virtual wstring FromDisplay();

	protected:

		mutex _mutex;

		CnxInfo _defaultIp;

		CnxInfo _vcsMediaData;

		CnxInfo _rtpRelayIp;

		int _rtpRelayTopPort;

		int _rtpRelayBottomPort;

		wstring _scriptFile;

		CodecsList _codecsList;

		wstring _from;

		wstring _fromDisplay;

	};

}


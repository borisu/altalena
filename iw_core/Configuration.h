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
#include "IwBase.h"

using namespace std;


namespace ivrworx
{

	class Configuration
		:public boost::noncopyable
	{
	public:

		Configuration(void);

		virtual ~Configuration(void);

		
		virtual bool Precompile();

		
		virtual string SuperScript();

		
		virtual string SuperMode();

		
		virtual CnxInfo IvrCnxInfo();

		
		virtual bool EnableSessionTimer();

		
		virtual string SipRefreshMode();

		
		virtual int	 SipDefaultSessionTime();
		

		
		virtual string ScriptFile();


		virtual string SoundsPath();


		virtual void AddMediaFormat(IN const MediaFormat& media_format);

		
		virtual void AddMediaFormat(IN const MediaFormat* codec);

		
		virtual const MediaFormatsPtrList& MediaFormats();

		
		virtual string From();

		
		virtual string FromDisplay();

		
		virtual string SyslogHost();

		
		virtual int SyslogPort();

		
		virtual string DebugLevel();

		
		virtual string DebugOutputs();

		
		virtual string ResipLog();

		
		virtual bool SyncLog();

		
		virtual int GetInt(const string &name) = 0;

		
		virtual string GetString(const string &name) = 0;

		
		virtual BOOL GetBool(const string &name) = 0;

	protected:

		mutex _mutex;

		bool _precomile;

		CnxInfo _ivrCnxInfo;

		string _scriptFile;

		MediaFormatsPtrList _mediaFormatPtrsList;

		string _from;

		string _fromDisplay;

		string _soundsPath;

		int _imsTopPort;

		int _imsBottomPort;

		string _sysloghost;

		int _syslogport;

		string _debugLevel;

		string _debugOutputs;

		string _resipLog;

		string _sipRefreshMode;

		int _sipDefaultSessionTime;

		bool _enableSessionTimer;

		string _superScript;

		string _superMode;

		bool _syncLog;

	};

}


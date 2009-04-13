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

	/**
	*	
	*	@defgroup configuration 
	*
	*	Configuration of ivrworx.
	*
	*	This is abstract class representing configuration of ivrworx.
	*	It does neither read values nor supplies default configuration 
	*	values.Every concrete class that implements the reading from 
	*	specific data source should inherit from this class. The class 
	*	is thread safe.
	*
	*/
	class Configuration
	{
	public:

		Configuration(void);

		virtual ~Configuration(void);

		/**
		*
		*	The connection info of SIP stack.
		*
		*	@return The connection info of SIP stack.
		**/
		virtual CnxInfo IvrCnxInfo();

		/**
		*	The connection info of IMS thread. The port is ignored.
		*
		*	@return The connection info of IMS thread.
		**/
		virtual CnxInfo ImsCnxInfo();

		/**
		*
		*	Top port of IMS ports range.
		*	
		*	@return Port number.
		**/
		virtual int ImsTopPort();

		/**
		*	Bottom port of IMS ports range.
		*
		*	@return Port number.
		**/
		virtual int ImsBottomPort();

		/**
		*
		*	Name of the lua file to run on each call.
		*
		*	@return Name of the lua file to run on each call.
		**/
		virtual string ScriptFile();

		/**
		*	Name of the lua file to run on each call. The script may be relative to 
		*	ivrworx installation directory.
		*
		*	@return pathname to script.
		**/
		virtual string SoundsPath();

		/**
		*	Adds supported media format.
		*
		*	@param_in Supported media format 
		**/
		virtual void AddMediaFormat(IN const MediaFormat& media_format);

		/**
		*	Adds supported media format.
		*
		*	@param_in Supported media format 
		**/
		virtual void AddMediaFormat(IN const MediaFormat* codec);

		/**
		*	Supported media formats list.
		*
		*	@return Supported media formats list.
		**/
		virtual const MediaFormatsPtrList& MediaFormats();

		/**
		*	This string will appear as a username in outgoing SIP calls.
		*
		*	@return User name.
		**/
		virtual string From();

		/**
		*	This string will appear as a display name of the user in outgoing 
		*	SIP calls.
		*
		*	@return Display name.
		**/
		virtual string FromDisplay();

		/**
		*	Syslogd host.
		*
		*	@return Syslogd hostname.
		**/
		virtual string SyslogHost();

		/**
		*	Syslogd port.
		*
		*	@return Syslogd port.
		**/
		virtual int SyslogPort();

		/**
		*	String that represent debug level for ivrworx infrastructure.
		*	Valid values: OFF|CRT|WRN|DBG|TRC
		*
		*	@return Syslogd port.
		**/
		virtual string DebugLevel();

		/**
		*	Comma separated values of log output facilities.
		*
		*	Currently supported:-
		*	- console - console output
		*	- debug - windows debug output
		*	- syslog - syslog daemon
		*
		*	Example: syslog,console
		*
		*	@return Comma separated values of log output facilities.
		**/
		virtual string DebugOutputs();

		/**
		*	Vertical bar "|" separated string of pairs representing the 
		*	debug level of resiprocate SIP stack subsystems.
		*
		*	pair syntax : SUBSYSTEM,DEBUG LEVEL
		*	Resiprocate log syntax: pair1|pair2|...
		*
		*	Valid subsystems: - APP,CONTENTS,DNS,DUM,NONE,PRESENCE,SDP,SIP,TRANSPORT,STATS
		*	Valid log values: the same as ivrworx
		*
		*	@return resip log levels
		**/
		virtual string ResipLog();

	protected:

		mutex _mutex;

		CnxInfo _ivrCnxInfo;

		CnxInfo _imsCnxInfo;

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

	};

}


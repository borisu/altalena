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

#ifdef _IW_TELEPHONY_API // assume this is defined when we build the DLL
#define IW_TELEPHONY_API  __declspec( dllexport)
#else
#define IW_TELEPHONY_API  __declspec( dllimport)
#endif

#define CALL_MSG_BASE		MSG_USER_DEFINED+1000
#define SIP_MSG_BASE		MSG_USER_DEFINED+2000
#define H323_MSG_BASE		MSG_USER_DEFINED+3000
#define RTSP_MSG_BASE		MSG_USER_DEFINED+4000
#define STREAM_MSG_BASE		MSG_USER_DEFINED+5000
#define RTP_PROXY_MSG_BASE	MSG_USER_DEFINED+6000

namespace ivrworx
{



/**

Denotes SDP format (speech codec, dtmf encoding..)

**/
class IW_TELEPHONY_API MediaFormat
{
public:

	enum MediaType
	{
		MediaType_UNKNOWN,
		MediaType_SPEECH,
		MediaType_DTMF,
		MediaType_CN
	};

	MediaFormat();

	MediaFormat(IN const MediaFormat &codec);

	MediaFormat(IN const MediaFormat *codec);

	MediaFormat(
		IN const string &param_name, 
		IN int param_sampling_rate,
		IN int param_sdp_mapping,
		IN MediaType media_type = MediaType_UNKNOWN);

	int sampling_rate() const;

	const string& sampling_rate_tos() const;

	int sdp_mapping() const;

	const string& sdp_mapping_tos() const;

	const string& sdp_name_tos() const;

	const string& get_sdp_a() const;

	MediaType get_media_type() const;

	const BOOL is_valid() const;

	static const MediaFormat UNKNOWN;

	static const MediaFormat PCMA;

	static const MediaFormat PCMU;

	static const MediaFormat DTMF_RFC2833;

	static const MediaFormat SPEEX;

	int operator == (const MediaFormat &other) const;

	int operator != (const MediaFormat &other) const;

	static MediaType GetMediaType(const string &name);

	static MediaFormat GetMediaFormat(const string &name);

private:

	void init_strings();

	int rate;

	int mapping;

	string rate_s;

	string mapping_s;

	string name;

	string sdp_a;

	MediaType media_type;

};

typedef list<const MediaFormat*> MediaFormatsPtrList;

typedef list<const MediaFormat> MediaFormatsList;

typedef map<int,const MediaFormat> MediaFormatsMap;

typedef pair<int,const MediaFormat> MediaFormatMapPair;

IW_TELEPHONY_API ostream& operator << (ostream &ostream, const MediaFormat &ptr);

class IW_TELEPHONY_API AbstractOffer
{
public:
	AbstractOffer(){};
	AbstractOffer(const string &body_, const string &type_):body(body_),type(type_){};
	string body;
	string type;

	string GetType() const
	{
		return AbstractOffer::Type(*this);
	}

	string GetSubType() const
	{
		return AbstractOffer::SubType(*this);
	}

	static string Type(const AbstractOffer &o);
	static string SubType(const AbstractOffer &o);

	static string Type(const string &s);
	static string SubType(const string &s);
};

class IW_TELEPHONY_API Credentials
{
public:
	string username;
	string password;
	string realm;

	Credentials(){};
	Credentials(const string &username_, const string &password_, const string &realm_)
	:username(username_),
	password(password_),
	realm(realm_){};

	bool isValid()
	{
		return (!username.empty() && !realm.empty());
	}
};

class SdpParserImpl;

class IW_TELEPHONY_API SdpParser
{
public:
	struct IW_TELEPHONY_API Medium
	{
		CnxInfo connection;

		MediaFormatsList list;

		MediaFormat dtmf_format;

		MediaFormat cn_format;

		void append_rtp_map(stringstream &str);

		void append_codec_list(stringstream &str);

	};

	SdpParser(IN const string &sdp);

	Medium first_audio_medium();

private:

	shared_ptr<SdpParserImpl> _impl;

};

};
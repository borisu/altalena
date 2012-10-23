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

#include "stdafx.h"
#include "Telephony.h"

using namespace resip;

namespace ivrworx
{
	class MediaFormat;

	typedef map<string,MediaFormat> 
		ConfFormatsMap;

	static ConfFormatsMap mf_map;

	MediaFormat::MediaFormat()
		:name("INVALID"),
		rate(IW_UNDEFINED),
		mapping(IW_UNDEFINED),
		media_type(MediaType_UNKNOWN)
	{
		init_strings();
	};

	MediaFormat::MediaFormat(
		IN const string &param_name, 
		IN int param_sampling_rate,
		IN int param_sdp_mapping, 
		IN MediaType _media_type)
		:name(param_name),
		rate(param_sampling_rate),
		mapping(param_sdp_mapping),
		media_type(_media_type)
	{
		if (media_type == MediaFormat::MediaType_UNKNOWN)
		{
			media_type = GetMediaType(param_name);
		}
		init_strings();
	};

	MediaFormat::MediaFormat(IN const MediaFormat *other)
	{
		name = other->name;
		rate = other->rate;
		mapping = other->mapping;
		media_type = other->media_type;

		init_strings();

	}

	MediaFormat::MediaFormat(IN const MediaFormat &other)
	{
		name = other.name;
		rate = other.rate;
		mapping = other.mapping;
		media_type = other.media_type;

		init_strings();

	};

	int MediaFormat::sampling_rate() const 
	{ 
		return rate; 
	}

	const string& MediaFormat::sampling_rate_tos() const 
	{ 
		return rate_s; 
	}

	int MediaFormat::sdp_mapping() const 
	{ 
		return mapping; 
	}

	const string& MediaFormat::sdp_mapping_tos() const 
	{ 
		return mapping_s; 
	}

	const string& MediaFormat::sdp_name_tos() const 
	{ 
		return name; 
	}


	const string& MediaFormat::get_sdp_a() const
	{
		return sdp_a;
	}

	MediaFormat::MediaType MediaFormat::get_media_type() const
	{
		return media_type;
	}

	const BOOL MediaFormat::is_valid() const
	{
		return media_type != MediaType_UNKNOWN;
	}

	void MediaFormat::init_strings()
	{

		// convert port
		char buffer[10];
		buffer[0] = '\0';

		// rate
		if ( _itoa_s(rate,buffer,10,10) != 0)
		{
			rate_s = "INVALID";
		} 
		else
		{
			rate_s = string(buffer);
		}

		// sdp mapping
		if ( _itoa_s(mapping,buffer,10,10) != 0)
		{
			mapping_s = "INVALID";
		} 
		else
		{
			mapping_s = string(buffer);
		}

		sdp_a   += "a=rtpmap:" + sdp_mapping_tos() + " "  + sdp_name_tos() + "/" + sampling_rate_tos() ;

	}

	int
		MediaFormat::operator ==(const MediaFormat &other) const
	{
		return (
			(other.mapping == this->mapping) &&
			(other.rate == this->rate) &&
			(other.name == this->name));
	}

	int
		MediaFormat::operator !=(const MediaFormat &other) const
	{
		return !(*this == other);
	}

	const MediaFormat MediaFormat::UNKNOWN;

	const MediaFormat MediaFormat::PCMU("PCMU",8000,0, MediaType_SPEECH);

	const MediaFormat MediaFormat::PCMA("PCMA",8000,8, MediaType_SPEECH);

	const MediaFormat MediaFormat::SPEEX("SPEEX",8000,97, MediaType_SPEECH);

	const MediaFormat MediaFormat::DTMF_RFC2833("telephone-event", 8000, 101, MediaType_DTMF);

	MediaFormat::MediaType  
		MediaFormat::GetMediaType(const string &media_name)
	{
		return GetMediaFormat(media_name).get_media_type();

	}

	MediaFormat 
	MediaFormat::GetMediaFormat(const string &name)
	{
		if (mf_map.empty())
		{
			mf_map["PCMA"]			  =  MediaFormat::PCMA;
			mf_map["PCMU"]			  =  MediaFormat::PCMU;
			mf_map["SPEEX"]			  =  MediaFormat::SPEEX;
			mf_map["telephone-event"] =  MediaFormat::DTMF_RFC2833;
		}


		ConfFormatsMap::iterator iter = mf_map.find(name);
		if (iter == mf_map.end())
		{
			return MediaFormat::UNKNOWN;
		}

		return iter->second;

	}

	IW_TELEPHONY_API ostream& operator << (ostream &ostream,  const MediaFormat &ptr)
	{
		return ostream << "name:" << ptr.sdp_name_tos()<< " rate:" << ptr.sampling_rate() << " payload:" << ptr.sdp_mapping();
	}

	IW_TELEPHONY_API string AbstractOffer::Type(const AbstractOffer &o)
	{
		return AbstractOffer::Type(o.type);
	}

	IW_TELEPHONY_API string AbstractOffer::SubType(const AbstractOffer &o)
	{
		int index = o.type.find_first_of("/");
		return index ==  string::npos || index == o.type.length()? 
			string("") : o.type.substr(index + 1, o.type.length() - (index+1));
	}


	IW_TELEPHONY_API string AbstractOffer::Type(const string &s)
	{
		int index = s.find_first_of("/");
		return index ==  string::npos || index == 0 ? s : s.substr(0, index);
	}

	IW_TELEPHONY_API string AbstractOffer::SubType(const string &s)
	{
		int index = s.find_first_of("/");
		return index ==  string::npos || index == s.length()? 
			string("") : s.substr(index + 1, s.length() - (index+1));
	}


	struct SdpParserImpl : 	public SdpContents
	{

		SdpParserImpl(HeaderFieldValue *hfv)
			:SdpContents(hfv,Mime("application","sdp")){};

		shared_ptr<HeaderFieldValue> hf_value;

	};

	void
	SdpParser::Medium::append_codec_list(stringstream &str)
	{
		for (MediaFormatsList::iterator iter = this->list.begin(); iter!=this->list.end(); iter++)
		{
			str << " " << iter->sdp_mapping();
		}
	}

	void
	SdpParser::Medium::append_rtp_map(stringstream &str)
	{
		for (MediaFormatsList::iterator iter = this->list.begin(); iter!=this->list.end(); iter++)
		{
			str << iter->get_sdp_a() << "\r\n";
		}
	}


	 
	SdpParser::SdpParser(const string &sdp)
	{
		// this is trick to ensure hfv makes an internal copy
		HeaderFieldValue temp(sdp.c_str(),sdp.size());
		HeaderFieldValue *value =  new HeaderFieldValue (temp);

		
		try
		{
			_impl.reset(new SdpParserImpl(value));
		} 
		catch (exception e)
		{
			LogWarn("SdpParser::SdpParser - error parsing e:" << e.what() <<  ", sdp:" << sdp);
			return;
		}
		

		// value should also be deleted!!!
		// sdp parser doe not own it
		_impl->hf_value.reset(value);


	}

	

	SdpParser::Medium
	SdpParser::first_audio_medium()
	{
		SdpParser::Medium res;

		if (!_impl)
			return res;

		const SdpContents::Session &s = _impl->session();

		if (!s.media().empty())
		{
			const Data &addr_data = s.connection().getAddress();

			// currently we only "audio" conversation
			list<SdpContents::Session::Medium>::const_iterator medium_iter = s.media().begin();
			for (;medium_iter != s.media().end();medium_iter++)
			{
				const SdpContents::Session::Medium &curr_medium = (*medium_iter);
				if (_stricmp("audio",curr_medium.name().c_str()) == 0)
					break;

			}

			if (medium_iter != s.media().end())
			{
				const SdpContents::Session::Medium &medium = *medium_iter;
				

				int port =	medium.port();

				std::list<SdpContents::Session::Connection> conns = medium.getConnections();
				res.connection = (conns.size() == 0) ?  CnxInfo(addr_data.c_str(),port) : 
					CnxInfo(conns.front().getAddress().c_str() ,port);
				

				// send list of codecs to the main process
				const list<Codec> &offered_codecs = medium.codecs();
				for (list<Codec>::const_iterator codec_iter = offered_codecs.begin(); 
					codec_iter != offered_codecs.end(); 
					codec_iter++)
				{
					if (strcmp(codec_iter->getName().c_str(),"CN") == 0)
					{
						res.cn_format = MediaFormat(
							codec_iter->getName().c_str(),
							codec_iter->getRate(),
							codec_iter->payloadType(),
							MediaFormat::MediaType_CN);
					} 
					else if (strcmp(codec_iter->getName().c_str(),"telephone-event") == 0)
					{
						res.dtmf_format = MediaFormat(
							codec_iter->getName().c_str(),
							codec_iter->getRate(),
							codec_iter->payloadType(),
							MediaFormat::MediaType_DTMF);
					}
					res.list.push_back(MediaFormat(
						codec_iter->getName().c_str(),
						codec_iter->getRate(),
						codec_iter->payloadType()));
				}
			}
		}

		return res;


	}

	
	



}
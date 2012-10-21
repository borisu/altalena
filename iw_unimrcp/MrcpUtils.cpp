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
#include "MrcpUtils.h"

namespace ivrworx {

	template<>
	void copy_param<unsigned int>(unsigned int *dest, 
		const char* key,
		int header_key, 
		MrcpParams *params,
		mrcp_message_t *mrcp_message,
		BOOL generic_resource)
	{

		MrcpParams::iterator iter = params->find(key);
		if (iter == params->end())
			return;

		string &v = iter->second;

		*dest = ::atoi(v.c_str());
		(generic_resource ? mrcp_generic_header_property_add: mrcp_resource_header_property_add)
			(mrcp_message,header_key);

	};


	template<>
	void copy_param<float>(float *dest, 
		const char* key,
		int header_key, 
		MrcpParams *params,
		mrcp_message_t *mrcp_message,
		BOOL generic_resource )
	{

		MrcpParams::iterator iter = params->find(key);
		if (iter == params->end())
			return;

		

		string &v = iter->second;

		*dest = (float)::atof(v.c_str());
	
		(generic_resource ? mrcp_generic_header_property_add: mrcp_resource_header_property_add)
			(mrcp_message,header_key);

	};



	template<>
	void copy_param<apt_str_t>(apt_str_t *dest, 
		const char* key,
		int header_key, 
		MrcpParams *params,
		mrcp_message_t *mrcp_message,
		BOOL generic_resource )
	{

		MrcpParams::iterator iter = params->find(key);
		if (iter == params->end())
			return;

		string &val = any_cast<string>(iter->second);
		apt_string_assign(dest,val.c_str(),mrcp_message->pool);

		(generic_resource ? mrcp_generic_header_property_add: mrcp_resource_header_property_add)
			(mrcp_message,header_key);


	};

	void translate_synth_params_into_message(
		mrcp_message_t *mrcp_message, 
		mrcp_synth_header_t *synth_header,
		MrcpParams *params)
	{
		/** MAY be specified in a CONTROL method and controls the
		amount to jump forward or backward in an active "SPEAK" request */
		copy_param(&synth_header->jump_size,"jump_size", SYNTHESIZER_HEADER_JUMP_SIZE, params,mrcp_message);
		/** MAY be sent as part of the "SPEAK" method to enable kill-
		on-barge-in support */
		copy_param(&synth_header->kill_on_barge_in,"kill_on_barge_in", SYNTHESIZER_HEADER_KILL_ON_BARGE_IN, params,mrcp_message);
		/** MAY be part of the "SET-PARAMS"/"GET-PARAMS" or "SPEAK"
		request from the client to the server and specifies a URI which
		references the profile of the speaker */
		copy_param(&synth_header->speaker_profile,"speaker_profile", SYNTHESIZER_HEADER_SPEAKER_PROFILE, params,mrcp_message);
		/** MUST be specified in a "SPEAK-COMPLETE" event coming from
		the synthesizer resource to the client */
		copy_param(&synth_header->completion_cause,"completion_cause", SYNTHESIZER_HEADER_COMPLETION_CAUSE, params,mrcp_message);
		/** MAY be specified in a "SPEAK-COMPLETE" event coming from
		the synthesizer resource to the client */
		copy_param(&synth_header->completion_reason,"completion_reason", SYNTHESIZER_HEADER_COMPLETION_REASON, params,mrcp_message);
		/** This set of headers defines the voice of the speaker */
		copy_param(&synth_header->voice_param,"voice_param", SYNTHESIZER_HEADER_VOICE_GENDER, params,mrcp_message);
		/** This set of headers defines the prosody of the speech */
		copy_param(&synth_header->prosody_param, "prosody_param", SYNTHESIZER_HEADER_PROSODY_VOLUME, params,mrcp_message);
		/** Contains timestamp information in a "timestamp" field */
		copy_param(&synth_header->speech_marker,"speech_marker", SYNTHESIZER_HEADER_SPEECH_MARKER, params,mrcp_message);
		/** specifies the default language of the speech data if the
		language is not specified in the markup */
		copy_param(&synth_header->speech_language,"speech_language", SYNTHESIZER_HEADER_SPEECH_LANGUAGE, params,mrcp_message);
		/** When the synthesizer needs to fetch documents or other resources like
		speech markup or audio files, this header controls the corresponding
		URI access properties */
		copy_param(&synth_header->fetch_hint,"fetch_hint", SYNTHESIZER_HEADER_FETCH_HINT, params,mrcp_message);
		/** When the synthesizer needs to fetch documents or other resources like
		speech audio files, this header controls the corresponding URI access
		properties */
		copy_param(&synth_header->audio_fetch_hint,"audio_fetch_hint", SYNTHESIZER_HEADER_AUDIO_FETCH_HINT, params,mrcp_message);
		/** When a synthesizer method needs a synthesizer to fetch or access a
		URI and the access fails, the server SHOULD provide the failed URI in
		this header in the method response */
		copy_param(&synth_header-> failed_uri,"failed_uri", SYNTHESIZER_HEADER_FAILED_URI, params,mrcp_message);
		/** When a synthesizer method needs a synthesizer to fetch or access a
		URI and the access fails the server MUST provide the URI specific or
		protocol specific response code for the URI in the Failed-URI header
		in the method response through this header */
		copy_param(&synth_header->failed_uri_cause,"failed_uri_cause", SYNTHESIZER_HEADER_FAILED_URI_CAUSE, params,mrcp_message);
		/** When a CONTROL request to jump backward is issued to a currently
		speaking synthesizer resource, and the target jump point is before
		the start of the current "SPEAK" request, the current "SPEAK" request
		MUST restart */
		copy_param(&synth_header->speak_restart,"speak_restart", SYNTHESIZER_HEADER_SPEAK_RESTART, params,mrcp_message);
		/** MAY be specified in a CONTROL method to control the
		length of speech to speak, relative to the current speaking point in
		the currently active "SPEAK" request */
		copy_param(&synth_header->speak_length,"speak_length", SYNTHESIZER_HEADER_SPEAK_LENGTH, params,mrcp_message);
		/** Used to indicate whether a lexicon has to be loaded or unloaded */
		copy_param(&synth_header->load_lexicon, "load_lexicon", SYNTHESIZER_HEADER_LOAD_LEXICON, params,mrcp_message);
		/** used to specify a list of active Lexicon URIs and the
		search order among the active lexicons */
		copy_param(&synth_header->lexicon_search_order,"lexicon_search_order", SYNTHESIZER_HEADER_LEXICON_SEARCH_ORDER, params,mrcp_message);

	};


	void translate_generic_params_into_message(
		mrcp_message_t *mrcp_message, 
		mrcp_generic_header_t *generic_header,
		MrcpParams *params)
	{
		/** Indicates the list of request-ids to which it should apply */
		//mrcp_request_id_list_t active_request_id_list;
		/** Helps the resource receiving the event, proxied by the client, 
		to decide if this event has been processed through a direct interaction of the resources */
		copy_param(&generic_header->proxy_sync_id,"proxy_sync_id", GENERIC_HEADER_PROXY_SYNC_ID, params,mrcp_message,TRUE);
		/** Specifies the acceptable character set for entities returned in the response or events associated with this request */
		copy_param(&generic_header->accept_charset, "accept_charset", GENERIC_HEADER_ACCEPT_CHARSET, params,mrcp_message,TRUE);
		/** Restricted to speech markup, grammar, recognition results, etc. */
		copy_param(&generic_header->content_type, "content_type", GENERIC_HEADER_CONTENT_TYPE, params,mrcp_message,TRUE);
		/** Contains an ID or name for the content, by which it can be referred to */
		copy_param(&generic_header->content_id, "content_id", GENERIC_HEADER_CONTENT_ID, params,mrcp_message,TRUE);
		/** May be used to specify the base URI for resolving relative URLs within the entity */
		copy_param(&generic_header->content_base,"content_base", GENERIC_HEADER_CONTENT_BASE, params,mrcp_message,TRUE);
		/** Indicates what additional content coding has been applied to the entity-body */
		copy_param(&generic_header->content_encoding,"content_encoding", GENERIC_HEADER_CONTENT_ENCODING, params,mrcp_message,TRUE);
		/** Statement of the location of the resource corresponding to this particular entity at the time of the request */
		copy_param(&generic_header->content_location,"content_location", GENERIC_HEADER_CONTENT_LOCATION, params,mrcp_message,TRUE);
		/** Contains the length of the content of the message body */
		//GetTableNumberParam<size_t>(L,-1,     gp.content_length,"content_length", params,mrcp_message,TRUE);
		/** Defines the default caching algorithms on the media server for the session or request */
		copy_param(&generic_header->cache_control,"cache_control", GENERIC_HEADER_CACHE_CONTROL, params,mrcp_message,TRUE);
		/** Sets the logging tag for logs generated by the media server */
		copy_param(&generic_header->logging_tag,"logging_tag", GENERIC_HEADER_LOGGING_TAG, params,mrcp_message,TRUE);
		/** Specifies the vendor specific parameters used by the media server */
		//apt_pair_arr_t        *vendor_specific_params;
		/** Additional headers for MRCP v2 */
		/** Specifies the acceptable media types set for entities returned in the response or events associated with this request */
		copy_param(&generic_header->accept,"accept", GENERIC_HEADER_ACCEPT, params,mrcp_message,TRUE);
		/** Defines the timeout for content that the server may need to fetch over the network */
		copy_param(&generic_header->fetch_timeout,"fetch_timeout", GENERIC_HEADER_FETCH_TIMEOUT, params,mrcp_message,TRUE);
		/** Enables to synchronize the cookie store of MRCP v2 client and server */
		copy_param(&generic_header->set_cookie,"set_cookie", GENERIC_HEADER_SET_COOKIE, params,mrcp_message,TRUE);
		/** Enables to synchronize the cookie store of MRCP v2 client and server */
		copy_param(&generic_header->set_cookie2,"set_cookie2", GENERIC_HEADER_SET_COOKIE2, params,mrcp_message,TRUE);

	};


	void translate_recog_params_into_message(
		mrcp_message_t *mrcp_message, 
		mrcp_recog_header_t *recog_header,
		MrcpParams *params)
	{

		/** Tells the recognizer resource what confidence level the client considers a
		successful match */
		copy_param(&recog_header->confidence_threshold,"confidence_threshold",RECOGNIZER_HEADER_CONFIDENCE_THRESHOLD, params,mrcp_message);
		/** To filter out background noise and not mistake it for speech */
		copy_param(&recog_header->sensitivity_level,"sensitivity_level",RECOGNIZER_HEADER_SENSITIVITY_LEVEL, params,mrcp_message);
		/** Tunable towards Performance or Accuracy */
		copy_param(&recog_header->speed_vs_accuracy,"speed_vs_accuracy",RECOGNIZER_HEADER_SPEED_VS_ACCURACY, params,mrcp_message);
		/** The client, by setting this header, can ask the recognition resource 
		to send it more  than 1 alternative */
		copy_param(&recog_header->n_best_list_length,"n_best_list_length",RECOGNIZER_HEADER_N_BEST_LIST_LENGTH, params,mrcp_message);
		/** The client can use the no-input-timeout header to set this timeout */
		copy_param(&recog_header->no_input_timeout,"no_input_timeout",RECOGNIZER_HEADER_NO_INPUT_TIMEOUT, params,mrcp_message);
		/** The client can use the recognition-timeout header to set this timeout */
		copy_param(&recog_header->recognition_timeout,"recognition_timeout",RECOGNIZER_HEADER_RECOGNITION_TIMEOUT, params,mrcp_message);
		/** MUST be present in the RECOGNITION-COMPLETE event if the Save-Waveform
		header was set to true */
		copy_param(&recog_header->waveform_uri,"waveform_uri",RECOGNIZER_HEADER_WAVEFORM_URI, params,mrcp_message);
		/** MUST be part of a RECOGNITION-COMPLETE, event coming from
		the recognizer resource to the client */
		copy_param(&recog_header->completion_cause, "completion_cause", RECOGNIZER_HEADER_COMPLETION_CAUSE,params,mrcp_message);
		/** MAY be sent as part of the SET-PARAMS or GET-PARAMS request */
		copy_param(&recog_header->recognizer_context_block,"recognizer_context_block", RECOGNIZER_HEADER_RECOGNIZER_CONTEXT_BLOCK, params,mrcp_message);
		/** MAY be sent as part of the RECOGNIZE request. A value of false tells
		the recognizer to start recognition, but not to start the no-input timer yet */
		copy_param(&recog_header->start_input_timers,"start_input_timers", RECOGNIZER_HEADER_START_INPUT_TIMERS, params,mrcp_message);
		/** Specifies the length of silence required following user
		speech before the speech recognizer finalizes a result */
		copy_param(&recog_header->speech_complete_timeout,"speech_complete_timeout", RECOGNIZER_HEADER_SPEECH_COMPLETE_TIMEOUT, params,mrcp_message);
		/** Specifies the required length of silence following user
		speech after which a recognizer finalizes a result */
		copy_param(&recog_header->speech_incomplete_timeout,"speech_incomplete_timeout", RECOGNIZER_HEADER_SPEECH_INCOMPLETE_TIMEOUT, params,mrcp_message);
		/** Specifies the inter-digit timeout value to use when
		recognizing DTMF input */
		copy_param(&recog_header->dtmf_interdigit_timeout,"dtmf_interdigit_timeout", RECOGNIZER_HEADER_DTMF_INTERDIGIT_TIMEOUT, params,mrcp_message);
		/** Specifies the terminating timeout to use when 
		recognizing DTMF input*/
		copy_param(&recog_header->dtmf_term_timeout,"dtmf_term_timeout", RECOGNIZER_HEADER_DTMF_TERM_TIMEOUT, params,mrcp_message);
		/** Specifies the terminating DTMF character for DTMF input
		recognition */
		copy_param(&recog_header->dtmf_term_char,"dtmf_term_char", RECOGNIZER_HEADER_DTMF_TERM_CHAR, params,mrcp_message);
		/** When a recognizer needs to fetch or access a URI and the access fails
		the server SHOULD provide the failed URI in this header in the method response*/
		copy_param(&recog_header->failed_uri,"failed_uri", RECOGNIZER_HEADER_FAILED_URI,  params,mrcp_message);
		/** When a recognizer method needs a recognizer to fetch or access a URI
		and the access fails the server MUST provide the URI specific or
		protocol specific response code for the URI in the Failed-URI header */
		copy_param(&recog_header->failed_uri_cause,"failed_uri_cause", RECOGNIZER_HEADER_FAILED_URI_CAUSE, params,mrcp_message);
		/** Allows the client to request the recognizer resource to
		save the audio input to the recognizer */
		copy_param(&recog_header->save_waveform, "save_waveform", RECOGNIZER_HEADER_SAVE_WAVEFORM, params,mrcp_message);
		/** MAY be specified in a RECOGNIZE request and allows the
		client to tell the server that, from this point on, further input
		audio comes from a different audio source */
		copy_param(&recog_header->new_audio_channel,"new_audio_channel",RECOGNIZER_HEADER_NEW_AUDIO_CHANNEL, params,mrcp_message);
		/** Specifies the language of recognition grammar data within
		a session or request, if it is not specified within the data */
		copy_param(&recog_header->speech_language,"speech_language",RECOGNIZER_HEADER_SPEECH_LANGUAGE, params,mrcp_message);
		/** Additional headers for MRCP v2 */
		/** Specifies if the input that caused a barge-in was DTMF or speech */
		copy_param(&recog_header-> input_type,"input_type", RECOGNIZER_HEADER_INPUT_TYPE,params,mrcp_message);
		/** Optional header specifies a URI pointing to audio content to be
		processed by the RECOGNIZE operation */
		copy_param(&recog_header->input_waveform_uri,"input_waveform_uri", RECOGNIZER_HEADER_INPUT_WAVEFORM_URI,params,mrcp_message);
		/** MAY be specified in a RECOGNITION-COMPLETE event coming from
		the recognizer resource to the client */
		copy_param(&recog_header->completion_reason,"completion_reason", RECOGNIZER_HEADER_COMPLETION_CAUSE,params,mrcp_message);
		/** tells the server resource the Media Type in which to store captured 
		audio such as the one captured and returned by the Waveform-URI header */
		copy_param(&recog_header->media_type,"media_type", RECOGNIZER_HEADER_MEDIA_TYPE, params,mrcp_message);
		/** lets the client request the server to buffer the
		utterance associated with this recognition request into a buffer
		available to a co-resident verification resource */
		copy_param(&recog_header->ver_buffer_utterance, "ver_buffer_utterance", RECOGNIZER_HEADER_VER_BUFFER_UTTERANCE, params,mrcp_message);
		/** Specifies what mode the RECOGNIZE method will operate in */
		copy_param(&recog_header->recognition_mode,"recognition_mode", RECOGNIZER_HEADER_RECOGNITION_MODE,params,mrcp_message);
		/** Specifies what will happen if the client attempts to
		invoke another RECOGNIZE method when this RECOGNIZE request is
		already in progress for the resource*/
		copy_param(&recog_header->cancel_if_queue,"cancel_if_queue", RECOGNIZER_HEADER_CANCEL_IF_QUEUE, params,mrcp_message);
		/** Specifies the maximum length of an utterance (in seconds) that will
		be considered for Hotword recognition */
		copy_param(&recog_header->hotword_max_duration,"hotword_max_duration", RECOGNIZER_HEADER_HOTWORD_MAX_DURATION, params,mrcp_message);
		/** Specifies the minimum length of an utterance (in seconds) that will
		be considered for Hotword recognition */
		copy_param(&recog_header->hotword_min_duration,"hotword_min_duration", RECOGNIZER_HEADER_HOTWORD_MAX_DURATION, params,mrcp_message);
		/** Provides a pointer to the text for which a natural language interpretation is desired */
		copy_param(&recog_header->interpret_text,"interpret_text",RECOGNIZER_HEADER_INTERPRET_TEXT, params,mrcp_message);
		/** MAY be specified in a GET-PARAMS or SET-PARAMS method and
		is used to specify the size in time, in milliseconds, of the
		typeahead buffer for the recognizer */
		copy_param(&recog_header->dtmf_buffer_time,"dtmf_buffer_time",RECOGNIZER_HEADER_DTMF_BUFFER_TIME, params,mrcp_message);
		/** MAY be specified in a RECOGNIZE method and is used to
		tell the recognizer to clear the DTMF type-ahead buffer before
		starting the recognize */
		copy_param(&recog_header->clear_dtmf_buffer,"clear_dtmf_buffer", RECOGNIZER_HEADER_CLEAR_DTMF_BUFFER, params,mrcp_message);
		/** MAY be specified in a RECOGNIZE method and is used to
		tell the recognizer that it MUST not wait for the end of speech
		before processing the collected speech to match active grammars */
		copy_param(&recog_header->early_no_match,"early_no_match", RECOGNIZER_HEADER_EARLY_NO_MATCH, params,mrcp_message);

	}

}
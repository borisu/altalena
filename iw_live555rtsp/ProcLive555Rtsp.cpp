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
#include "ProcLive555Rtsp.h"
#include "Live555RtspSession.h"

namespace ivrworx
{
	
	int GenerateNewRtsph()
	{
		static long rtsph_counter = 0;

		return ::InterlockedExchangeAdd(&rtsph_counter,1);
	}
	
	ProcLive555Rtsp::ProcLive555Rtsp(ConfigurationPtr conf,LpHandlePair pair):
	LightweightProcess(pair,"ProcLive555Rtsp"),
	_conf(conf),
	_rtspClient(NULL),
	_session(NULL)
	{
		static const size_t buf_len = 1024;
		char buf[buf_len];
		_snprintf_s(buf,buf_len,"proto=rtsp;vendor=live555;uid=%d",_processId);

		ServiceId(buf);

		// currently it is thread per client so we generate the 
		// handle only once
		_rtspHandle = GenerateNewRtsph();
	}

	ProcLive555Rtsp::~ProcLive555Rtsp(void)
	{

	}

	void
	ProcLive555Rtsp::real_run()
	{
		FUNCTRACKER;

		_scheduler	=	BasicTaskScheduler::createNew();
		_env  =	IwUsageEnvironment::createNew(*_scheduler);

		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while(shutdown_flag == FALSE)
		{

			ApiErrorCode res = API_SUCCESS;
			IwMessagePtr msg = _inbound->Wait(Seconds(180), res);

			if (res == API_TIMEOUT)
			{
				LogDebug("ProcLive555Rtsp::real_run - Keep Alive rtsph:")
				continue;
			}

			switch (msg->message_id)
			{
			case MSG_RTSP_SETUP_SESSION_REQ:
				{
					SetupSession(msg);
					break;
				}
			case MSG_RTSP_PLAY_REQ:
				{
					Play(msg);
					break;
				}
			case MSG_RTSP_PAUSE_REQ:
				{
					Pause(msg);
					break;
				}
			case MSG_RTSP_TEARDOWN_REQ:
				{
					Teardown(msg);
					break;
				}
			case MSG_PROC_SHUTDOWN_REQ:
				{
					shutdown_flag = TRUE;
					SendResponse(msg,new MsgShutdownAck());
					break;
				}
			default:
				{
					BOOL res = HandleOOBMessage(msg);
					if (res == FALSE)
					{
						LogWarn("ProcLive555Rtsp::real_run received unknown message id:" << msg->message_id_str);
					}
				} // default
			} // switch
		} // while


	}

	RTSPClient* createClient(UsageEnvironment& env,
		int verbosityLevel, char const* applicationName) {
			portNumBits tunnelOverHTTPPortNum = 0;
			return RTSPClient::createNew(env, verbosityLevel, applicationName,
				tunnelOverHTTPPortNum);
	}

	TaskToken sessionTimerTask = NULL;

	void sessionTimerHandler(void* /*clientData*/) 
	{

	}

	char* 
	getOptionsResponse(
		Medium* client, 
		char const* url,
		char* username, 
		char* password) 
	{
		RTSPClient* rtspClient = (RTSPClient*)client;
		return rtspClient->sendOptionsCmd(url, username, password);
	}

	char* 
	getSDPDescriptionFromURL(
		Medium* client, 
		char const* url,
		char const* username, char const* password,
		char const* /*proxyServerName*/,
		unsigned short /*proxyServerPortNum*/,
		unsigned short /*clientStartPort*/) 
	{
		
		RTSPClient* rtspClient = (RTSPClient*)client;
		char* result;
		if (username != NULL && password != NULL) {
			result = rtspClient->describeWithPassword(url, username, password);
		} 
		else 
		{
				result = rtspClient->describeURL(url);
		}

		unsigned statusCode = 0;
		statusCode = rtspClient->describeStatus();
		return result;
	}

	Boolean 
	clientSetupSubsession(
		Medium* client, 
		MediaSubsession* subsession,
		Boolean streamUsingTCP) 
	{
		if (client == NULL || subsession == NULL) 
		{
			return False;
		}

		RTSPClient* rtspClient = (RTSPClient*)client;
		return rtspClient->setupMediaSubsession(*subsession,
			False, streamUsingTCP);
	}

	Boolean clientStartPlayingSession(
		Medium* client,
		MediaSession* session,
		double duration,
		double initialSeekTime,
		double scale) 
	{
			double endTime = initialSeekTime;
			if (scale > 0) {
				if (duration <= 0) endTime = -1.0f;
				else endTime = initialSeekTime + duration;
			} else {
				endTime = initialSeekTime - duration;
				if (endTime < 0) endTime = 0.0f;
			}

			if (client == NULL || session == NULL) return False;
			RTSPClient* rtspClient = (RTSPClient*)client;
			return rtspClient->playMediaSession(*session, initialSeekTime, endTime, (float)scale);
	}


	Boolean
	setupStreams(
		RTSPClient *ourClient,
		MediaSession *session, 
		Boolean streamUsingTCP, 
		UsageEnvironment *env) 
	{
		MediaSubsessionIterator iter(*session);
		MediaSubsession *subsession;
		Boolean madeProgress = False;

		while ((subsession = iter.next()) != NULL) {
			if (subsession->clientPortNum() == 0) continue; // port # was not set

			if (!clientSetupSubsession(ourClient, subsession, streamUsingTCP)) {
				LogWarn("Failed to setup \"" << subsession->mediumName()
					<< "/" << subsession->codecName()
					<< "\" subsession: " << env->getResultMsg());
			} else {
				LogDebug("Setup \"" << subsession->mediumName()
					<< "/" << subsession->codecName()
					<< "\" subsession (client ports " << subsession->clientPortNum()
					<< "-" << subsession->clientPortNum()+1 << ")");
				madeProgress = True;
			}
		}
		return madeProgress;
	}

	Boolean
	startPlayingStreams(
		RTSPClient *ourClient,
	    MediaSession *session, 
		UsageEnvironment *env,
		double duration = 0,
		double durationSlop = -1.0, // extra seconds to play at the end
		double initialSeekTime = 0.0f,
		double scale = 1.0f) 
	{
		if (duration == 0) {
			if (scale > 0) duration = session->playEndTime() - initialSeekTime; // use SDP end time
			else if (scale < 0) duration = initialSeekTime;
		}
		if (duration < 0) duration = 0.0;

		if (!clientStartPlayingSession(	ourClient,session,duration,initialSeekTime,scale)) {
			LogWarn("Failed to start playing session: " << env->getResultMsg());
			return False;
		} else {
			LogDebug("Started playing session");
			return True;
		}
//
//		IF YOU WANT TO IMPLEMENT QOS GO TO ORIGINAL LIVE555 testProgs SOURCE	
//
// 		if (qosMeasurementIntervalMS > 0) {
// 			// Begin periodic QOS measurements:
// 			beginQOSMeasurement();
// 		}

		// Figure out how long to delay (if at all) before shutting down, or
		// repeating the playing
		Boolean timerIsBeingUsed = False;
		double secondsToDelay = duration;
		if (duration > 0) {
			double const maxDelayTime
				= (double)( ((unsigned)0x7FFFFFFF)/1000000.0 );
			if (duration > maxDelayTime) {
				LogWarn("Warning: specified end time " << duration
					<< " exceeds maximum " << maxDelayTime
					<< "; will not do a delayed shutdown");
			} else {

				timerIsBeingUsed = True;
				double absScale = scale > 0 ? scale : -scale; // ASSERT: scale != 0
				secondsToDelay = duration/absScale + durationSlop;

				int uSecsToDelay = (int)(secondsToDelay*1000000.0);
				sessionTimerTask = env->taskScheduler().scheduleDelayedTask(
					uSecsToDelay, (TaskFunc*)sessionTimerHandler, (void*)NULL);
			}
		}

		if (timerIsBeingUsed) 
		{
			LogDebug ("Data is being streamed"
				<< " (for up to " << secondsToDelay
				<< " seconds)...");
		} 

//
//		IF YOU WANT TO IMPLEMENT QOS GO TO ORIGINAL LIVE555 testProgs SOURCE	
//
// 		// Watch for incoming packets (if desired):
// 		checkForPacketArrival(NULL);
// 		checkInterPacketGaps(NULL);
	}

	void 
	ProcLive555Rtsp::Teardown()
	{
		FUNCTRACKER;

		if (_rtspClient)
		{
			if (_session)
			{
				_rtspClient->teardownMediaSession(*_session);
				MediaSession::close(_session);
				_session = NULL;
			}

			Medium::close(_rtspClient);
			_rtspClient = NULL;
		}

		
	}



	void 
	ProcLive555Rtsp::SetupSession(IwMessagePtr msg)
	{
		FUNCTRACKER;

 		shared_ptr<MsgRtspSetupSessionReq>
 			req = dynamic_pointer_cast<MsgRtspSetupSessionReq>(msg);

		// safe side
		Teardown();

		_rtspClient = 
			createClient(*_env,1,"RtspClient");

		SdpParser p(req->offer.body);
		SdpParser::Medium audio_medium = p.first_audio_medium();

		
		char* sdpDescription
			= getSDPDescriptionFromURL(
			_rtspClient, 
			req->request_url.c_str(), 
			NULL, // username, 
			NULL, // password,
			NULL, // proxyServerName, 
			0,	  // proxyServerPortNum,
			554);
	

		if (sdpDescription == NULL) 
		{
			LogWarn("ProcLive555Rtsp::SetupSession - Failed to get a SDP description from URL \"" << req->request_url
				<< " live555 msg:" << *_env->getResultMsg());
			SendResponse(msg, new MsgRtspSetupSessionNack());
			return;
		}

		// Create a media session object from this SDP description:
		MediaSession *session = 
			MediaSession::createNew(*_env, sdpDescription);

		delete[] sdpDescription;
		if (session == NULL) {
			LogWarn("ProcLive555Rtsp::SetupSession - Failed to create a MediaSession object from the SDP description: " << *_env->getResultMsg());
			SendResponse(msg, new MsgRtspSetupSessionNack());
			return;
		} else if (!session->hasSubsessions()) {
			LogWarn("ProcLive555Rtsp::SetupSession - This session has no media subsessions (i.e., \"m=\" lines)");
			SendResponse(msg, new MsgRtspSetupSessionNack());
			return;
		}

		char *singleMedium="audio";

		// Then, setup the "RTPSource"s for the session:
		MediaSubsessionIterator iter(*session);
		MediaSubsession *curr_subsession = NULL;
		MediaSubsession *subsession_candidate = NULL;

		MediaFormat &media_format = (*audio_medium.list.begin());
		// find if there is an appropriate codec
		while ((curr_subsession = iter.next()) != NULL) 
		{
			if (strcmp(curr_subsession->mediumName(),"audio") == 0 &&
				media_format.sdp_name_tos() == curr_subsession->codecName() )
			{
				LogDebug("ProcLive555Rtsp::SetupSession - chosen :" << curr_subsession->controlPath());
				subsession_candidate = curr_subsession;
				break;
			}
		} // while

		if (curr_subsession == NULL)
		{
			LogWarn("ProcLive555Rtsp::SetupSession - This session has no media subsessions with needed codec:" << media_format.sdp_name_tos());
			SendResponse(msg, new MsgRtspSetupSessionNack());
			return;
		}

		
		// setting port will indicate that this session is in use
		subsession_candidate->setClientPortNum(audio_medium.connection.port_ho());
		Boolean res = setupStreams(_rtspClient,session,False,_env);
		if (res == False)
		{
			LogWarn("ProcLive555Rtsp::SetupSession - Error setting up session");
			SendResponse(msg, new MsgRtspSetupSessionNack());
		}

		_session = session;
		MsgRtspSetupSessionAck *ack = new MsgRtspSetupSessionAck();
		ack->rtsp_handle = _rtspHandle;
		ack->offer.type = "application/sdp";
		ack->offer.body = curr_subsession->savedSDPLines();

		SendResponse(msg, ack);


	}


	void 
	ProcLive555Rtsp::Play(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgRtspPlayReq>
			req = dynamic_pointer_cast<MsgRtspPlayReq>(msg);

		if (!_rtspClient || 
			!_session)
		{
			SendResponse(msg, new MsgRtspPlayNack());
			return;
		}

		Boolean res = startPlayingStreams(_rtspClient,_session,_env,req->duration,-1.0,req->start_time,req->scale);
		if (res == TRUE)
		{
			SendResponse(msg,new MsgRtspPlayAck());
		}
		else
		{
			SendResponse(msg,new MsgRtspPlayNack());
		}
		
	}

	void 
	ProcLive555Rtsp::Teardown(IwMessagePtr msg)
	{
		FUNCTRACKER;

		Teardown();

	}

	void 
	ProcLive555Rtsp::Pause(IwMessagePtr msg)
	{
		FUNCTRACKER;

		if (!_rtspClient || 
			!_session)
		{
			SendResponse(msg, new MsgRtspPauseNack());
			return;
		}

		Boolean res = _rtspClient->pauseMediaSession(*_session);
		if (res == TRUE)
		{
			SendResponse(msg,new MsgRtspPauseAck());
		}
		else
		{
			SendResponse(msg,new MsgRtspPauseNack());
		}

	}

}


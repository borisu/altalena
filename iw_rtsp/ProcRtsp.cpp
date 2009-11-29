#include "StdAfx.h"
#include "ProcRtsp.h"
#include "RtspSession.h"

namespace ivrworx
{
	ProcRtsp::ProcRtsp(Configuration &conf,LpHandlePair pair):
	LightweightProcess(pair,"ProcRtsp"),
	_conf(conf)
	{

	}

	ProcRtsp::~ProcRtsp(void)
	{

	}

	void
	ProcRtsp::real_run()
	{
		FUNCTRACKER;

		_scheduler	=	BasicTaskScheduler::createNew();
		_env  =	BasicUsageEnvironment::createNew(*_scheduler);

		AllocateSession(IwMessagePtr());

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
	ProcRtsp::AllocateSession(IwMessagePtr msg)
	{
		FUNCTRACKER;

// 		shared_ptr<MsgRtspAllocateSessionReq>
// 			req = dynamic_pointer_cast<MsgRtspAllocateSessionReq>(msg);

		RTSPClient* ourClient = 
			createClient(*_env,1,"RtspClient");

		char *url = "rtsp://spongebob/please-hold.wav";

		int desiredPortNum = 1234;

		char* sdpDescription
			= getSDPDescriptionFromURL(
			ourClient, 
			url, 
			NULL, // username, 
			NULL, // password,
			NULL, // proxyServerName, 
			desiredPortNum, // proxyServerPortNum,
			554);

		

		if (sdpDescription == NULL) 
		{
			LogWarn("Failed to get a SDP description from URL \"" << url
				<< "\": " << *_env->getResultMsg());
			return;
		}

		// Create a media session object from this SDP description:
		MediaSession *session = 
			MediaSession::createNew(*_env, sdpDescription);

		delete[] sdpDescription;
		if (session == NULL) {
			LogWarn("Failed to create a MediaSession object from the SDP description: " << *_env->getResultMsg());
			throw;
		} else if (!session->hasSubsessions()) {
			LogWarn("This session has no media subsessions (i.e., \"m=\" lines)");
			throw;
		}

		char *singleMedium="audio";

		// Then, setup the "RTPSource"s for the session:
		MediaSubsessionIterator iter(*session);
		MediaSubsession *subsession;
		Boolean madeProgress = False;
		char const* singleMediumToTest = singleMedium;//singleMedium;
		while ((subsession = iter.next()) != NULL) {
			// If we've asked to receive only a single medium, then check this now:
			if (singleMediumToTest != NULL) {
				if (strcmp(subsession->mediumName(), singleMediumToTest) != 0) {
					LogWarn ("Ignoring \"" << subsession->mediumName()
						<< "/" << subsession->codecName()
						<< "\" subsession, because we've asked to receive a single " << singleMedium
						<< " session only");
					continue;
				} else {
					// Receive this subsession only
					singleMediumToTest = "xxxxx";
					// this hack ensures that we get only 1 subsession of this type
				}
			}

			if (desiredPortNum != 0) {
				subsession->setClientPortNum(desiredPortNum);
				desiredPortNum += 2;
			}

		}

		setupStreams(ourClient,session,False,_env);

		startPlayingStreams(ourClient,session,_env);

		int x;
		cin >> x;


	}

}


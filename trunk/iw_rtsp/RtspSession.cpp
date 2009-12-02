#include "StdAfx.h"
#include "RtspSession.h"
#include "ProcRtsp.h"

namespace ivrworx
{
	RtspSession::RtspSession(ScopedForking &forking, Configuration &conf):
	_forking(forking),
	_rtspHandle(IW_UNDEFINED),
	_conf(conf)
	{
		

		
	}

	RtspSession::~RtspSession(void)
	{
		FUNCTRACKER;

		if (_rtspProcPair.inbound)
		{
			GetCurrLightWeightProc()->Shutdown(Seconds(5),_rtspProcPair);
		}
	}

	ApiErrorCode
	RtspSession::Init()
	{
		FUNCTRACKER;

		// due to limitation of live555 (synchronous) 
		// we currently open thread per session, sad but true
		DECLARE_NAMED_HANDLE_PAIR(rtsp_proc_pair);
		_rtspProcPair = rtsp_proc_pair;

		_forking.fork(new ProcRtsp(_conf,rtsp_proc_pair));

		ApiErrorCode res = GetCurrLightWeightProc()->WaitTillReady(Seconds(5),_rtspProcPair);
		return res;

	}

	ApiErrorCode 
	RtspSession::Setup(IN const string &rtsp_url, IN const MediaFormat &format)
	{

		FUNCTRACKER;

		LogDebug("RtspSession::Setup - rtsp_url:" << rtsp_url  << ", codec:"  << format << ", rtsh:" <<  _rtspHandle);

		MsgRtspSetupSessionReq *msg = new MsgRtspSetupSessionReq();
		msg->request_url = rtsp_url;
		msg->media_format = format;

		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
			_rtspProcPair.inbound,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
			"RTSP Setup TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("RtspSession::Setup - Error allocating RTSP session" << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTSP_SETUP_SESSION_ACK:
			{


				shared_ptr<MsgRtspSetupSessionAck> ack = 
					shared_polymorphic_cast<MsgRtspSetupSessionAck>(response);

				_rtspHandle	= ack->rtsp_handle;

				LogDebug("RtspSession::Setup - RTSP session allocated successfully, rtsph:" << _rtspHandle);

				break;

			}
		case MSG_RTSP_SETUP_SESSION_NACK:
			{
				LogDebug("Error allocating Rtsp session.");
				res = API_SERVER_FAILURE;
				break;
			}
		default:
			{
				throw;
			}
		}

		return res;

	}

	ApiErrorCode 
	RtspSession::Play(double start_time /* = 0.0 */, double end_time /* = 0.0  */, double scale /* = 1.0 */)
	{
		LogDebug("RtspSession::Play - start_time:" << start_time  << ", end_time:"  << end_time << ", scale: scale" << ", rtsh:" <<  _rtspHandle);

		if (_rtspHandle == IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		MsgRtspPlayReq *msg = new MsgRtspPlayReq();
		msg->start_time = start_time;
		msg->end_time = end_time;
		msg->scale = scale;

		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
			_rtspProcPair.inbound,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
			"RTSP Play TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("RtspSession::Play - Error playing RTSP session" << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTSP_PLAY_ACK:
			{
				break;
			}
		case MSG_RTSP_SETUP_SESSION_NACK:
			{
				LogWarn("RtspSession::Play - Error allocating Rtsp session.");
				res = API_SERVER_FAILURE;
				break;
			}
		default:
			{
				throw;
			}
		}

		return res;
	}

	ApiErrorCode 
	RtspSession::Pause()
	{
		LogDebug("RtspSession::Pause rtsh:" <<  _rtspHandle);

		if (_rtspHandle == IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		MsgRtspPauseReq *msg = new MsgRtspPauseReq();
		
		IwMessagePtr response = NULL_MSG;
		ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
			_rtspProcPair.inbound,
			IwMessagePtr(msg),
			response,
			MilliSeconds(GetCurrLightWeightProc()->TransactionTimeout()),
			"RTSP Pause TXN");

		if (res != API_SUCCESS)
		{
			LogWarn("RtspSession::Pause - Error playing RTSP session" << res);
			return res;
		}

		switch (response->message_id)
		{
		case MSG_RTSP_PAUSE_ACK:
			{
				break;
			}
		case MSG_RTSP_PAUSE_NACK:
			{
				LogWarn("RtspSession::Pause - Error allocating Rtsp session.");
				res = API_SERVER_FAILURE;
				break;
			}
		default:
			{
				throw;
			}
		}

		return res;
	}

	ApiErrorCode 
	RtspSession::TearDown()
	{
		FUNCTRACKER;

		LogDebug("RtspSession::TearDown rtsph:" <<  _rtspHandle);

		if (_rtspHandle == IW_UNDEFINED)
		{
			return API_FAILURE;
		}

		GetCurrLightWeightProc()->SendMessage(_rtspProcPair.inbound, 
			IwMessagePtr(new MsgRtspTearDownReq()));

		return API_SUCCESS;

	}


}

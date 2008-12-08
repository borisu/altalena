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
#include "StreamingObject.h"
#include "CcuLogger.h"

#define CCU_IMS_DEFAULT_SAMPLING_RATE 8000
#define CCU_IMS_DEFAULT_CHANNEL_NUMBER 1


StreamingObject::StreamingObject(IN const CnxInfo &remote_media_data, 
								 IN const wstring &file_name,
								 IN ImsHandleId handle_id):
_remoteMediaData(remote_media_data),
_fileName(file_name),
_interval(0.020), // We'll use 20 millisecond intervals.
_timer(_interval),
_chain("IMS Streamer Chain"),
_iteration(1),
_handleId(handle_id)
{
	FUNCTRACKER;

	LogDebug(">>Created<< StreamingObject uid=[" << GetObjectUid() << "] dest=[" << remote_media_data << "] , file=[" << file_name << "]");
	
}

StreamingObject::~StreamingObject(void)
{
	FUNCTRACKER;

	LogDebug(">>Destroyed<< StreamingObject uid=[" << GetObjectUid() << "]");
}


CnxInfo 
StreamingObject::RemoteMediaData() const 
{ 
	return _remoteMediaData; 
}

void 
StreamingObject::RemoteMediaData(CnxInfo val) 
{ 
	_remoteMediaData = val; 
}

ImsHandleId
StreamingObject::ImsHandle()
{
	return _handleId;
}

CcuApiErrorCode
StreamingObject::Process()
{
	bool done = false;
	bool error = false;
	string error_component;
	string error_message;

#pragma TODO("Do we have to do poll for transmitting only connection?")	
	int status = 0;
// 	int status = _rtpSession.Poll();
// 	if (status != 0)
// 	{
// 		LogWarn(">>Error<< polling RTPSession res=[" << status << "] msg=[" << RTPGetErrorString(status) << "]");
// 		return CCU_API_FAILURE;
// 	}

	MIPSystemMessage startMsg(MIPSYSTEMMESSAGE_TYPE_ISTIME);
	_chain.process(
		done,
		error,
		error_component,
		error_message,
		_iteration,
		startMsg);

	if (error)
	{
		LogWarn(">>Error<< streaming object component=[" << StringToWString(error_component) << "] msg=[" << StringToWString(error_message) << "]");
	}

	if (error)
	{
		return CCU_API_FAILURE;
	}

	if (done)
	{
		return CCU_API_SUCCESS;
	}

	return CCU_API_OPERATION_IN_PROGRESS;
	
}

int 
StreamingObject::Port() const 
{ 
	return _port; 
}

CcuApiErrorCode 
StreamingObject::Close(PortManager &portManager)
{
	if (!_chain.stop())
	{
		return CCU_API_FAILURE;
	}

	portManager.MarkAvailable(Port());
	return CCU_API_SUCCESS;
}



CcuApiErrorCode
StreamingObject::Init(PortManager &portManager)
{
	
	CcuApiErrorCode res = CCU_API_SUCCESS;

	do {

		// We'll open the file 'soundfile.wav'.
		//
		bool mipBoolRetValue = _sndFileInput.open(
			WStringToString(_fileName),		// file name
			_interval,						// polling interval
			false							// loop
			);
		
		if (!mipBoolRetValue)
		{
			LogWarn("Cannot read file=[" <<_fileName << "] err=[" <<  StringToWString(_sndFileInput.getErrorString()) << "]");
			res = CCU_API_FAILURE;
			break;
		}


		// We'll convert to a sampling rate of 8000Hz and mono sound.
		//
		int samplingRate = CCU_IMS_DEFAULT_SAMPLING_RATE;
		int numChannels = CCU_IMS_DEFAULT_CHANNEL_NUMBER;

		mipBoolRetValue = _sampConv.init(samplingRate, numChannels);
		if (!mipBoolRetValue)
		{
			LogWarn("Cannot init sample converter sampling rate=[" <<  samplingRate << "] channels=[" << numChannels <<"]  err=[" <<  StringToWString(_sampConv.getErrorString()) << "]" );
			res = CCU_API_FAILURE;
			break;
		}


		// Initialize the sample encoder: the RTP U-law audio encoder
		// expects native endian signed 16 bit samples.
		//
		int destType = MIPRAWAUDIOMESSAGE_TYPE_S16;
		mipBoolRetValue = _sampEnc.init(destType);
		if (!mipBoolRetValue)
		{
			LogWarn("Cannot init sample encoder dest type=[" << destType << "] err=[" <<  StringToWString(_sampEnc.getErrorString()) << "]");
			res = CCU_API_FAILURE;
			break;
		}

		// Convert samples to U-law encoding
		mipBoolRetValue = _uLawEnc.init();
		if (!mipBoolRetValue)
		{
			LogWarn("Cannot create U-LAW encoder err=[" <<  StringToWString(_uLawEnc.getErrorString()) << "]");
			res = CCU_API_FAILURE;
			break;
		}

		// Initialize the RTP audio encoder: this component will create
		// RTP messages which can be sent to the RTP component.
		mipBoolRetValue = _rtpEnc.init();
		if (!mipBoolRetValue)
		{
			LogWarn("Cannot create U-LAW encoder err=[" <<  StringToWString(_rtpEnc.getErrorString()) << "]");
			res = CCU_API_FAILURE;
			break;
		}


		// We'll initialize the RTPSession object which is needed by the
		// RTP component.
		RTPUDPv4TransmissionParams transmissionParams;
		RTPSessionParams sessionParams;
		int status = 0;

		sessionParams.SetUsePollThread(false);


		//
		// find next available port and create RTP session
		//
		portManager.BeginSearch();

		int port_candidate = portManager.GetNextCandidate();
		bool found = false;

		while (port_candidate != CCU_UNDEFINED && (!found))
		{
			transmissionParams.SetPortbase(port_candidate);
			sessionParams.SetOwnTimestampUnit(1.0/((double)CCU_IMS_DEFAULT_SAMPLING_RATE));
			sessionParams.SetMaximumPacketSize(64000);
			sessionParams.SetAcceptOwnPackets(false);

			status = _rtpSession.Create(sessionParams,&transmissionParams);
			switch (status)
			{
			case 0:
				{
					portManager.MarkUnavailable(port_candidate);
					found = true;
					break;
				}
			case ERR_RTP_UDPV4TRANS_CANTBINDRTPSOCKET:
				{
					LogWarn("Socket " << port_candidate << " appears to be bound see if there's strayed RTP sessions or some other process has taken oner IMS port range.");
					portManager.MarkUnavailable(port_candidate);
					port_candidate = portManager.GetNextCandidate();
					continue;
				}
			default:
				{
					LogWarn("Cannot create RTP Session status=[" << status << "] desc=[" << StringToWString(RTPGetErrorString(status)) << "]");

					res = CCU_API_FAILURE;
					break;
				}
			}
		}

		if (port_candidate == CCU_UNDEFINED)
		{
			res = CCU_API_FAILURE;
			break;
		}

		_port = port_candidate;


		// Instruct the RTP session to send data to destination.
		status = _rtpSession.AddDestination(
			RTPIPv4Address(
			_remoteMediaData.iaddr_ho(),
			_remoteMediaData.port_ho()));
		if ( status != 0)
		{
			LogWarn("Error adding destination dest=[" << _remoteMediaData << "] description=[" << StringToWString(RTPGetErrorString(status)) << "]");
			res = CCU_API_FAILURE;
			break;
		}

		// Tell the RTP component to use this RTPSession object.
		status = _rtpComp.init(&_rtpSession);
		if (mipBoolRetValue == false)
		{
			LogWarn("Error initiating RTP component description=[" << StringToWString(RTPGetErrorString(status)) << "]");
			res = CCU_API_FAILURE;
			break;
		}

		//
		// Build the chain
		//

#define CHAIN_CHECK_ADDING_ERROR if (mipBoolRetValue == false) { LogWarn("Error adding connection"); res = CCU_API_FAILURE;	break;}

		mipBoolRetValue = _chain.setChainStart(&_sndFileInput);
		CHAIN_CHECK_ADDING_ERROR;

		mipBoolRetValue = _chain.addConnection(&_sndFileInput, &_sampConv);
		CHAIN_CHECK_ADDING_ERROR;

		mipBoolRetValue = _chain.addConnection(&_sampConv, &_sampEnc);
		CHAIN_CHECK_ADDING_ERROR;

		mipBoolRetValue = _chain.addConnection(&_sampEnc, &_uLawEnc);
		CHAIN_CHECK_ADDING_ERROR;

		mipBoolRetValue = _chain.addConnection(&_uLawEnc, &_rtpEnc);
		CHAIN_CHECK_ADDING_ERROR;

		mipBoolRetValue = _chain.addConnection(&_rtpEnc, &_rtpComp);
		CHAIN_CHECK_ADDING_ERROR;

#undef CHAIN_CHECK_ADDING_ERROR 	

		_chain.ExitOnEmptyChain(true);
		_chain.ClientThread(true);
		
		mipBoolRetValue = _chain.start();
		if (mipBoolRetValue == false)
		{
			res = CCU_API_FAILURE;
			break;
		}

	} while (false);

	if (CCU_FAILURE(res) &&  _port != CCU_UNDEFINED)
	{
		portManager.MarkAvailable(_port);
		_port = CCU_UNDEFINED;
	}

	return res;

}

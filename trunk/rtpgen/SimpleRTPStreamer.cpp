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
#include "SimpleRTPStreamer.h"
#include "CcuLogger.h"

#define RETURN_ON_FAILURE( X ) if (( X ) == false) return CCU_API_FAILURE




bool checkError(bool returnValue, const MIPComponent &component)
{
	if (returnValue == true)
		return true;

	LogWarn("An error occured in component: " << StringToWString(component.getComponentName()) << endl);
	LogWarn("Error description: " << StringToWString(component.getErrorString()) << endl);

	return false;
}

bool checkError(int status)
{
	if (status >= 0)
		return true;
	
	LogWarn("An error occured in the RTP component: " << endl);
	LogWarn("Error description: " << StringToWString(RTPGetErrorString(status)) << endl);
	
	return false;
}


bool checkError(bool returnValue, const MIPComponentChain &chain)
{
	if (returnValue == true)
		return true;

	LogWarn ("An error occured in chain: " << StringToWString(chain.getName()) << endl);
	LogWarn ("Error description: " << StringToWString(chain.getErrorString()) << endl);

	return false;
}

class MyChain : public MIPComponentChain
{
public:
	MyChain(const std::string &chainName) : MIPComponentChain(chainName)
	{
	}
private:
	void onThreadExit(bool error, const std::string &errorComponent, const std::string &errorDescription)
	{
		if (!error)
			return;

		LogWarn( "An error occured in the background thread." << endl);
		LogWarn( "    Component: " << StringToWString(errorComponent) << endl);
		LogWarn( "    Error description: " << StringToWString(errorDescription) << endl);
	}	
};


SimpleRTPStreamer::SimpleRTPStreamer(void):
_playTime(0)
{
}

SimpleRTPStreamer::~SimpleRTPStreamer(void)
{
}

long
SimpleRTPStreamer::playTime()
{
	return _playTime;
}

CcuApiErrorCode
SimpleRTPStreamer::SyncPlay(string file_name, CnxInfo data)
{

	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);


	MIPTime interval(0.020); // We'll use 20 millisecond intervals.
	MIPAverageTimer timer(interval);
	MIPWAVInput sndFileInput;
	MIPSamplingRateConverter sampConv, sampConv2;
	MIPSampleEncoder sampEnc, sampEnc2, sampEnc3;
	MIPULawEncoder uLawEnc;
	MIPRTPULawEncoder rtpEnc;
	MIPRTPComponent rtpComp;
	
	
	MyChain chain("Sound file player");
	RTPSession rtpSession;
	bool returnValue;

	// We'll open the file 'soundfile.wav'.

	returnValue = sndFileInput.open(file_name, interval,false);
	
	RETURN_ON_FAILURE(checkError(returnValue, sndFileInput));
	
	// We'll convert to a sampling rate of 8000Hz and mono sound.
	
	int samplingRate = 8000;
	int numChannels = 1;

	returnValue = sampConv.init(samplingRate, numChannels);
	RETURN_ON_FAILURE(checkError(returnValue, sampConv));

	// Initialize the sample encoder: the RTP U-law audio encoder
	// expects native endian signed 16 bit samples.
	
	returnValue = sampEnc.init(MIPRAWAUDIOMESSAGE_TYPE_S16);
	RETURN_ON_FAILURE(checkError(returnValue, sampEnc));

	// Convert samples to U-law encoding
	returnValue = uLawEnc.init();
	RETURN_ON_FAILURE(checkError(returnValue, uLawEnc));

	// Initialize the RTP audio encoder: this component will create
	// RTP messages which can be sent to the RTP component.

	returnValue = rtpEnc.init();
	RETURN_ON_FAILURE(checkError(returnValue, rtpEnc));

	// We'll initialize the RTPSession object which is needed by the
	// RTP component.
	
	RTPUDPv4TransmissionParams transmissionParams;
	RTPSessionParams sessionParams;
	int portBase = 60066;
	int status;

	transmissionParams.SetPortbase(portBase);
	sessionParams.SetOwnTimestampUnit(1.0/((double)samplingRate));
	sessionParams.SetMaximumPacketSize(64000);
	sessionParams.SetAcceptOwnPackets(false);
	
	
	status = rtpSession.Create(sessionParams,&transmissionParams);
	RETURN_ON_FAILURE(checkError(status));

	// Instruct the RTP session to send data to ourselves.
	status = rtpSession.AddDestination(
		RTPIPv4Address(data.iaddr_ho(),data.port_ho()));
	RETURN_ON_FAILURE(checkError(status));

	// Tell the RTP component to use this RTPSession object.
	returnValue = rtpComp.init(&rtpSession);
	RETURN_ON_FAILURE(checkError(returnValue, rtpComp));
	
	// Next, we'll create the chain
	returnValue = chain.setChainStart(&timer);
	RETURN_ON_FAILURE(checkError(returnValue, chain));

	returnValue = chain.addConnection(&timer, &sndFileInput);
	RETURN_ON_FAILURE(checkError(returnValue, chain));

	returnValue = chain.addConnection(&sndFileInput, &sampConv);
	RETURN_ON_FAILURE(checkError(returnValue, chain));

	returnValue = chain.addConnection(&sampConv, &sampEnc);
	RETURN_ON_FAILURE(checkError(returnValue, chain));

	returnValue = chain.addConnection(&sampEnc, &uLawEnc);
	RETURN_ON_FAILURE(checkError(returnValue, chain));
	
	returnValue = chain.addConnection(&uLawEnc, &rtpEnc);
	RETURN_ON_FAILURE(checkError(returnValue, chain));

	returnValue = chain.addConnection(&rtpEnc, &rtpComp);
	RETURN_ON_FAILURE(checkError(returnValue, chain));
	
	
	LogDebug("Started >>streaming<< file=[" << StringToWString(file_name) << "] to=[" << data << "]")
	long start = ::GetCurrentTime();
	returnValue = chain.start();
	RETURN_ON_FAILURE(checkError(returnValue, chain));

	// Poll for eof

	while (1)
	{
		::Sleep(1000);
		if (sndFileInput.m_eof) 
		{
			break;
		}
	}

	returnValue = chain.stop();
	checkError(returnValue, chain);

	rtpSession.Destroy();
	
	// We'll let the destructors of the other components take care
	// of their de-initialization.


	WSACleanup();

	_playTime = ::GetCurrentTime() - start;

	return CCU_API_SUCCESS;
}




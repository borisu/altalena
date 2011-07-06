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

#define DUMMY_SINK_BUFFER_SIZE (6+65535) /* large enough for a PES packet */

namespace ivrworx
{
	class MockRtpSink : public MediaSink {
	public:

		MockRtpSink(UsageEnvironment& env);
		virtual ~MockRtpSink();

		char watchVariable;

	private:
		// redefined virtual function:
		virtual Boolean continuePlaying();

	private:
		static void afterGettingFrame(void* clientData, unsigned frameSize,
			unsigned numTruncatedBytes,
		struct timeval presentationTime,
			unsigned durationInMicroseconds);
		void afterGettingFrame1();

	private:
		Boolean fReturnFirstSeenCode;
		unsigned char fBuf[DUMMY_SINK_BUFFER_SIZE];
	};



}


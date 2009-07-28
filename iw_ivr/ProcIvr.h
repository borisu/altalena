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


using namespace std;

namespace ivrworx
{

	class ProcIvr :
		public LightweightProcess
	{

	public:

		ProcIvr(IN LpHandlePair pair, IN Configuration &conf);

		virtual void real_run();

		virtual ~ProcIvr(void);

	protected:

		BOOL ProcessStackMessage(
			IN IwMessagePtr event,
			IN ScopedForking &forking
			);

		BOOL ProcessInboundMessage(
			IN IwMessagePtr event,
			IN ScopedForking &forking
			);

		void StartScript(
			IN IwMessagePtr msg);

	private:

		LpHandlePair _stackPair;

		CnxInfo _sipStackData;

		Configuration &_conf;

		char *_precompiledBuffer_Super;
		size_t _superSize;

		char *_precompiledBuffer;
		size_t _scriptSize;

		BOOL _waitingForSuperCompletion;

	};

}






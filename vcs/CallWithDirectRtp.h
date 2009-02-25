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

namespace ivrworx
{
	class CallWithDirectRtp :
		public Call
	{
	public:

		CallWithDirectRtp(
			IN LpHandlePair stack_pair,
			IN ScopedForking &forking,
			IN shared_ptr<MsgCallOfferedReq> offered_msg);

		virtual ~CallWithDirectRtp(void);

		virtual IxApiErrorCode AcceptCall();

		virtual IxApiErrorCode PlayFile(IN const wstring &file_name);

		virtual IxApiErrorCode WaitForDtmf(OUT int &dtmf, IN Time timeout);

	private:

		ImsSession _imsSession;

		shared_ptr<MsgCallOfferedReq> _origOffereReq;

	};


}

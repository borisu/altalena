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
#include "Message.h"

namespace ivrworx
{
	enum SqlEvts
	{
		MSG_SQL_OPEN_CONNECTION_REQ = MSG_USER_DEFINED,
		MSG_SQL_OPEN_CONNECTION_ACK
	};

	
	class MsgSqlOpenConnectionReq: 
		public MsgRequest
	{

	public:
		MsgSqlOpenConnectionReq():
		  MsgRequest(MSG_SQL_OPEN_CONNECTION_REQ, NAME(MSG_SQL_OPEN_CONNECTION_REQ)){};

		  string connection_url;
	};


	class MsgSqlOpenConnectionAck: 
		public MsgResponse
	{

	public:
		MsgSqlOpenConnectionAck():
		  MsgResponse(MSG_SQL_OPEN_CONNECTION_ACK, NAME(MSG_SQL_OPEN_CONNECTION_ACK)),
		  db(NULL), 
		  rc(IW_UNDEFINED){};

		  sqlite3 *db;

		  int rc;

	};


	class SqliteSession
	{
	public:
		SqliteSession(void);
		virtual ~SqliteSession(void);

		ApiErrorCode OpenConnection(const string &connection_url, sqlite3 **db, int &rc);

		ApiErrorCode Finalize();

		ApiErrorCode Step();

		ApiErrorCode Compile();

		ApiErrorCode Reset();

		



	};


}


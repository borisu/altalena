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
#include "SqliteSession.h"
#include "LightweightProcess.h"

namespace ivrworx
{
	SqliteSession::SqliteSession(void)
	{

	}

	SqliteSession::~SqliteSession(void)
	{
	}

	ApiErrorCode
	SqliteSession::OpenConnection(const string &connection_url, sqlite3 **db, int &rc)
	{

		MsgSqlOpenConnectionReq *open_req 
			= new MsgSqlOpenConnectionReq();

		open_req->connection_url = connection_url;
		
		IwMessagePtr response;
		ApiErrorCode res = GetCurrLightWeightProc()->DoRequestResponseTransaction(
			SQLITE_Q,
			IwMessagePtr(open_req),
			response,
			Seconds(10),
			"Open SQLite Db TXN"
			);

#pragma TODO ("possible memory leak is when txn times out but db was open")
		if (IW_FAILURE(res))
		{
			return res;
		};

		shared_ptr<MsgSqlOpenConnectionAck> ack 
			= shared_dynamic_cast<MsgSqlOpenConnectionAck> (response);

		*db = ack->db;
		rc	= ack->rc;

		return API_SUCCESS;


	}

}


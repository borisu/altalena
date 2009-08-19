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
#include "ProcSqlite.h"
#include "SqliteSession.h"
#include "Logger.h"

namespace  ivrworx
{
	ProcSqlite::ProcSqlite(Configuration &conf, LpHandlePair pair)
		:LightweightProcess(pair,"SQLite"),
		_conf(conf)
	{
	}

	ProcSqlite::~ProcSqlite(void)
	{
	}

	void
	ProcSqlite::real_run()
	{

		I_AM_READY;

		BOOL shutdown_flag = FALSE;
		while(shutdown_flag == FALSE)
		{

			ApiErrorCode res = API_SUCCESS;
			IwMessagePtr event = _inbound->Wait( Seconds(60), res);
				
			switch (res)
			{
			case API_TIMEOUT:
				{
					LogInfo("Sqlite keep alive.");
					continue;
				}
			case API_FAILURE:
				{
					LogCrit("Error wating for messages");
					return;
				}
			case API_SUCCESS:
				{
					break;
				}
			default:
				{

				}
			}

			switch (event->message_id)
			{
			case MSG_PROC_SHUTDOWN_REQ:
				{
					return;
				}

			case MSG_SQL_OPEN_CONNECTION_REQ:
				{
					UponSqlOpenConnectionReq(event);
					break;
				}
			default:
				{
					LogWarn("Unknown message");

				}
			}
		} //while
	}


	void
	ProcSqlite::UponSqlOpenConnectionReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		sqlite3 *db = NULL;

		shared_ptr<MsgSqlOpenConnectionReq> open_conn_req
			= shared_dynamic_cast<MsgSqlOpenConnectionReq>(msg);

		int rc = sqlite3_open(
			open_conn_req->connection_url.c_str(), 
			&db);
		
		MsgSqlOpenConnectionAck *ack = new MsgSqlOpenConnectionAck();
		ack->db = db;
		ack->rc = rc;

		SendResponse(msg, ack);


	}

}



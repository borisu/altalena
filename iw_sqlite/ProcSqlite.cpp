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
	ProcSqlite::ProcSqlite(ConfigurationPtr conf, LpHandlePair pair)
		:LightweightProcess(pair,"SQLite"),
		_conf(conf)
	{
		ServiceId(_conf->GetString("sqlite/uri"));
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
			IwMessagePtr msg = _inbound->Wait( Seconds(60), res);
				
			switch (res)
			{
			case API_TIMEOUT:
				{
					LogInfo("Sqlite keep alive.");
					continue;
				}
			case API_SUCCESS:
				{
					break;
				}
			default:
				{
					LogCrit("Error wating for messages");
					shutdown_flag = TRUE;
					continue;;
				}
			}

			switch (msg->message_id)
			{
			case MSG_PROC_SHUTDOWN_REQ:
				{
					SendResponse(msg,new MsgShutdownAck());
					shutdown_flag = TRUE;
					continue;
				}
			case MSG_SQL_OPEN_CONNECTION_REQ:
				{
					UponSqlOpenConnectionReq(msg);
					break;
				}
			case MSG_SQL_CLOSE_CONNECTION_REQ:
				{
					UponSqlCloseConnectionReq(msg);
					break;
				}
			case MSG_SQL_FINALIZE_REQ:
				{
					UponSqlFinalizeReq(msg);
					break;
				}
			case MSG_SQL_EXEC_REQ:
				{
					UponSqlExecReq(msg);
					break;
				}
			case MSG_SQL_STEP_REQ:
				{
					UponSqlStepReq(msg);
					break;
				}
			default:
				{
					BOOL res = HandleOOBMessage(msg);
					if (res == FALSE)
					{
						LogCrit("Unknown msg:" << msg->message_id);
						shutdown_flag = TRUE;
						continue;;
					}
				}
			}
		} //while

		CloseAllConnections();
	}

	void
	ProcSqlite::CloseAllConnections()
	{
		FUNCTRACKER;

		for (SqlSessionsMap::iterator iter = _sqlMap.begin(); 
			iter != _sqlMap.end(); ++iter)
		{
			::sqlite3_close((*iter).second);
		}

	}

	void
	ProcSqlite::UponSqlCloseConnectionReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgSqlCloseConnectionReq> close_conn_req
			= shared_dynamic_cast<MsgSqlCloseConnectionReq>(msg);

		SqlSessionsMap::iterator iter = 
			_sqlMap.find(close_conn_req->session_id);

		if (iter == _sqlMap.end())
		{
			return;
		}

		int res = sqlite3_close((*iter).second);
		LogDebug("sqlite3_close sqls:" << close_conn_req->session_id << " res:" << res);

	}

	void
	ProcSqlite::UponSqlFinalizeReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgSqlFinalizeReq> finalize_req
			= shared_dynamic_cast<MsgSqlFinalizeReq>(msg);

		SqlSessionsMap::iterator iter = _sqlMap.find(finalize_req->session_id);
		if ( iter == _sqlMap.end())
		{
			return;
		}

		int rc = sqlite3_finalize(finalize_req->pStmt);
		LogDebug("sqlite3_finalize sqls:" << finalize_req->session_id << " res:" << rc);

	}

	void
	ProcSqlite::UponSqlExecReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgSqlExecReq> exec_req
			= shared_dynamic_cast<MsgSqlExecReq>(msg);

		SqlSessionsMap::iterator iter = _sqlMap.find(exec_req->session_id);
		if ( iter == _sqlMap.end())
		{
			MsgSqlExecNack *nack = new MsgSqlExecNack();
			SendResponse(msg, nack);
			return;
		}

		sqlite3 *db = (*iter).second;

		char *errmsg;
		int rc = sqlite3_exec(
			db,
			exec_req->sql_statement.c_str(), 
			NULL,
			NULL,
			&errmsg);

		LogDebug("sqlite3_exec sqls:" << exec_req->session_id << " stmnt:" << exec_req->sql_statement <<" res:" << rc);

		MsgSqlOpenConnectionAck *ack = new MsgSqlOpenConnectionAck();
		ack->db = db;
		ack->rc = rc;
		ack->rc = rc;

		SendResponse(msg, ack);


	}

	void
	ProcSqlite::UponSqlStepReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		shared_ptr<MsgSqlStepReq> step_req
			= shared_dynamic_cast<MsgSqlStepReq>(msg);

		if (step_req->session_id < 0 ||
			_sqlMap.find(step_req->session_id) == _sqlMap.end())
		{
			MsgSqlStepNack *nack = new MsgSqlStepNack();
			SendResponse(msg, nack);
			return;
		}

		int rc = sqlite3_step(step_req->pStmt);
		LogDebug("sqlite3_step sqls:" << step_req->session_id << " res:" << rc);

		MsgSqlStepAck *ack = new MsgSqlStepAck();
		ack->rc = rc;
		SendResponse(msg, ack);



	}


	void
	ProcSqlite::UponSqlOpenConnectionReq(IwMessagePtr msg)
	{
		FUNCTRACKER;

		sqlite3 *db = NULL;

		shared_ptr<MsgSqlOpenConnectionReq> open_conn_req
			= shared_dynamic_cast<MsgSqlOpenConnectionReq>(msg);

		if (open_conn_req->session_id < 0 ||
			_sqlMap.find(open_conn_req->session_id) != _sqlMap.end())
		{
			MsgSqlOpenConnectionNack *nack = new MsgSqlOpenConnectionNack();
			SendResponse(msg, nack);
			return;
		}

		int rc = sqlite3_open(
			open_conn_req->connection_url.c_str(), 
			&db);

		LogDebug("sqlite3_open db:" << open_conn_req->connection_url.c_str() << " res:" << rc);

		if (db != NULL)
		{
			_sqlMap[open_conn_req->session_id] = db;
		}

			
		MsgSqlOpenConnectionAck *ack = new MsgSqlOpenConnectionAck();
		ack->db = db;
		ack->rc = rc;

		SendResponse(msg, ack);


	}

}



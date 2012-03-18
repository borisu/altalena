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
#include "Logger.h"


namespace ivrworx
{
	static HandleId GenerateNewSqlSessionId() 
	{
		static HandleId session_counter = 0;

		return ::InterlockedExchangeAdd((LONG*)(&session_counter),1);
	}

	SqliteSession::SqliteSession(void):
	_sessionId(IW_UNDEFINED)
	{

	}

	sqlite3_int64 
	SqliteSession::sqlite3_last_insert_rowid()
	{
		FUNCTRACKER;

		return ::sqlite3_last_insert_rowid(_db);
	}

	int 
	SqliteSession::sqlite3_changes()
	{
		FUNCTRACKER;

		return ::sqlite3_changes(_db);
	}

	int 
    SqliteSession::sqlite3_busy_timeout(int ms)
	{
		FUNCTRACKER;

		return ::sqlite3_busy_timeout(_db, ms);
	}

	ApiErrorCode 
	SqliteSession::sqlite3_prepare( 
		const char *zSql,       
		int nByte,              
		sqlite3_stmt **ppStmt,  
		const char **pzTail,
		int &res
	)
	{
		FUNCTRACKER;

		res  =  ::sqlite3_prepare(_db,zSql,nByte,ppStmt,pzTail);

		return API_SUCCESS;

	}

	const char *
	SqliteSession::sqlite3_errmsg()
	{
		FUNCTRACKER;

		return ::sqlite3_errmsg(_db);
	}

	void
	SqliteSession::sqlite3_close()
	{
		FUNCTRACKER;

		if (_sessionId == IW_UNDEFINED)
		{
			return;
		}
		
		MsgSqlCloseConnectionReq *close_req 
			= new MsgSqlCloseConnectionReq();

		close_req->session_id = _sessionId;

		ApiErrorCode res = GetCurrRunningContext()->SendMessage(
			SQLITE_Q,
			IwMessagePtr(close_req));

		_sessionId = IW_UNDEFINED;
	}

	SqliteSession::~SqliteSession(void)
	{
		FUNCTRACKER;

		sqlite3_close();
		
	}

	ApiErrorCode
	SqliteSession::sqlite3_open(const char *connection_url, int &rc)
	{

		FUNCTRACKER;

		if (_sessionId != IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		MsgSqlOpenConnectionReq *open_req 
			= new MsgSqlOpenConnectionReq();

		open_req->connection_url = connection_url;
		_sessionId = GenerateNewSqlSessionId();
		open_req->session_id = _sessionId;
		
		
		IwMessagePtr response;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			SQLITE_Q,
			IwMessagePtr(open_req),
			response,
			Seconds(10),
			"Open SQLite Db TXN"
			);

		if (IW_FAILURE(res))
		{
			return res;
		};

		switch (response->message_id)
		{
		case MSG_SQL_OPEN_CONNECTION_NACK:
			{
				return API_SERVER_FAILURE;
			}
		case MSG_SQL_OPEN_CONNECTION_ACK:
			{
				shared_ptr<MsgSqlOpenConnectionAck> ack 
					= shared_dynamic_cast<MsgSqlOpenConnectionAck> (response);

				_db = ack->db;
				rc	= ack->rc;

				return API_SUCCESS;
			}
		default:
			{
				return API_UNKNOWN_RESPONSE;
			}
		}
	}

	ApiErrorCode
	SqliteSession::sqlite3_exec( 
		const char *sql,                           /* SQL to be evaluated */
		int (*callback)(void*,int,char**,char**),  /* Callback function */
		void *,                                    /* 1st argument to callback */
		char **errmsg,                             /* Error msg written here */
		int &rc									   /* OUT: sqlite result */	
	)
	{

		FUNCTRACKER;

		if (_sessionId == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		if (callback != NULL)
		{
			LogCrit("Not supported sqlite interface option (callback)");
			throw;
		}

		MsgSqlExecReq *exec_req 
			= new MsgSqlExecReq();

		exec_req->sql_statement = sql;
		exec_req->session_id = _sessionId;
		

		IwMessagePtr response;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			SQLITE_Q,
			IwMessagePtr(exec_req),
			response,
			Seconds(10),
			"Exec SQLite Db TXN"
			);

		if (IW_FAILURE(res))
		{
			return res;
		};

		switch (response->message_id)
		{
		case MSG_SQL_EXEC_NACK:
			{
				return API_SERVER_FAILURE;
			}
		case MSG_SQL_EXEC_ACK:
			{
				shared_ptr<MsgSqlExecAck> ack 
					= shared_dynamic_cast<MsgSqlExecAck> (response);
				
				rc	= ack->rc;

				return API_SUCCESS;
			}
		default:
			{
				return API_UNKNOWN_RESPONSE;
			}
		}
	}


	ApiErrorCode
	SqliteSession::sqlite3_reset( 
		sqlite3_stmt *pStmt,
		int &rc						   
		)
	{

		FUNCTRACKER;

		if (_sessionId == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		rc = ::sqlite3_reset(pStmt);

		return API_SUCCESS;
	}

	ApiErrorCode
	SqliteSession::sqlite3_finalize( 
		sqlite3_stmt *pStmt,
		int &rc						   
		)
	{
	
		FUNCTRACKER;

		if (_sessionId == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		MsgSqlFinalizeReq *finalize_req 
			= new MsgSqlFinalizeReq();

		finalize_req->pStmt = pStmt;
		finalize_req->session_id = _sessionId;

		ApiErrorCode res = GetCurrRunningContext()->SendMessage(
			SQLITE_Q,
			IwMessagePtr(finalize_req));

		return res;

	}

	ApiErrorCode
	SqliteSession::sqlite3_step( 
		sqlite3_stmt *pStmt,
		int &rc						   
		)
	{

		FUNCTRACKER;

		if (_sessionId == IW_UNDEFINED)
		{
			return API_WRONG_STATE;
		}

		MsgSqlStepReq *step_req 
			= new MsgSqlStepReq();

		step_req->pStmt = pStmt;
		step_req->session_id = _sessionId;


		IwMessagePtr response;
		ApiErrorCode res = GetCurrRunningContext()->DoRequestResponseTransaction(
			SQLITE_Q,
			IwMessagePtr(step_req),
			response,
			Seconds(10),
			"Step SQLite Db TXN"
			);

		if (IW_FAILURE(res))
		{
			return res;
		};

		switch (response->message_id)
		{
		case MSG_SQL_STEP_NACK:
			{
				return API_SERVER_FAILURE;
			}
		case MSG_SQL_STEP_ACK:
			{
				shared_ptr<MsgSqlStepAck> ack 
					= shared_dynamic_cast<MsgSqlStepAck> (response);

				rc	= ack->rc;

				return API_SUCCESS;
			}
		default:
			{
				return API_UNKNOWN_RESPONSE;
			}
		}
	}
}


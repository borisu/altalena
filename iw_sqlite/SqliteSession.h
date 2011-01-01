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
#include "sqlite3.h"
#include "SqlFactory.h"

/**
@defgroup sql SQL Connectivity

@section Lua Scripts SQL Connectivity

In order to supply fiber non blocking SQL connectivity to the scripts, 
<A HREF="http://www.keplerproject.org/luasql/">Lua SQL</A> libarary was 
extended with sqlite driver. Of cause, you may use any lua sql driver you like, but running long sql 
queries will block all fibers in the main thread.

IwWorx sqlite driver outsources the long operation to a different thread by thus making fiber
reschedule possible while waiting for results. Any api available for lua sql is available for
ivrworx driver also.

Working with sqlite db is as simple as

@code
 env = assert(luasql.sqlite3());
 con = assert(env:connect("voice.db"))
 cur = assert(con:execute"SELECT * FROM users")

 row = cur:fetch ({}, "a")
 while row do
   ivrworx.loginf(string.format("user: %s, password: %s", row.login, row.password))
   -- reusing the table of results
   row = cur:fetch (row, "a")
 end
 -- close everything
 cur:close()
 con:close()
 env:close()
@endcode


**/

#define SQLITE_Q 4

namespace ivrworx
{
	enum SqlEvts
	{
		MSG_SQL_OPEN_CONNECTION_REQ = MSG_USER_DEFINED,
		MSG_SQL_OPEN_CONNECTION_ACK,
		MSG_SQL_OPEN_CONNECTION_NACK,
		MSG_SQL_CLOSE_CONNECTION_REQ,
		MSG_SQL_EXEC_REQ,
		MSG_SQL_EXEC_ACK,
		MSG_SQL_EXEC_NACK,
		MSG_SQL_STEP_REQ,
		MSG_SQL_STEP_ACK,
		MSG_SQL_STEP_NACK,
		MSG_SQL_FINALIZE_REQ,
		MSG_SQL_RESET_REQ
	};


	class IW_SQLLITE_API MsgSqlMixin
	{
	public:

		MsgSqlMixin(): db(NULL),rc(IW_UNDEFINED),session_id(IW_UNDEFINED){};

		struct sqlite3 *db;

		int rc;

		int session_id;

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgSqlMixin *req = 
				dynamic_cast<MsgSqlMixin*>(request);

			db		   = req->db != NULL ? req->db : db;
			session_id = req->session_id != IW_UNDEFINED ? req->session_id : session_id ;
		}

	};

	
	class IW_SQLLITE_API MsgSqlOpenConnectionReq: 
		public MsgSqlMixin, public MsgRequest
	{
	public:
		MsgSqlOpenConnectionReq():
		  MsgRequest(MSG_SQL_OPEN_CONNECTION_REQ, NAME(MSG_SQL_OPEN_CONNECTION_REQ)){};

		  string connection_url;

		 
	};

	class IW_SQLLITE_API MsgSqlOpenConnectionAck: 
		public MsgSqlMixin, public MsgResponse
	{
	public:
		MsgSqlOpenConnectionAck():
		  MsgResponse(MSG_SQL_OPEN_CONNECTION_ACK, NAME(MSG_SQL_OPEN_CONNECTION_ACK)){};

		 virtual void copy_data_on_response(IN IwMessage *request)
		 {
			  MsgSqlMixin::copy_data_on_response(request);
			  MsgResponse::copy_data_on_response(request);
		 }


	};

	class IW_SQLLITE_API MsgSqlOpenConnectionNack: 
		public MsgSqlMixin, public MsgResponse
	{
	public:
		MsgSqlOpenConnectionNack():
		  MsgResponse(MSG_SQL_OPEN_CONNECTION_NACK, NAME(MSG_SQL_OPEN_CONNECTION_NACK)){};

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgSqlMixin::copy_data_on_response(request);
			  MsgResponse::copy_data_on_response(request);
		  }

	};

	class IW_SQLLITE_API MsgSqlCloseConnectionReq: 
		public MsgSqlMixin, public MsgRequest
	{
	public:
		MsgSqlCloseConnectionReq():
		  MsgRequest(MSG_SQL_CLOSE_CONNECTION_REQ, NAME(MSG_SQL_CLOSE_CONNECTION_REQ)){};

	};

	class IW_SQLLITE_API MsgSqlExecReq: 
		public MsgSqlMixin, public MsgRequest
	{
	public:
		MsgSqlExecReq():
		  MsgRequest(MSG_SQL_EXEC_REQ, NAME(MSG_SQL_EXEC_REQ)){};

		string sql_statement;

	};

	class IW_SQLLITE_API MsgSqlExecAck: 
		public MsgSqlMixin, public MsgResponse
	{
	public:
		MsgSqlExecAck():
		  MsgResponse(MSG_SQL_EXEC_ACK, NAME(MSG_SQL_EXEC_ACK)),errmsg(NULL){};

		char *errmsg;

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgSqlMixin::copy_data_on_response(request);
			MsgResponse::copy_data_on_response(request);
		}

	};

	class IW_SQLLITE_API MsgSqlExecNack: 
		public MsgSqlMixin, public MsgResponse
	{
	public:
		MsgSqlExecNack():
		  MsgResponse(MSG_SQL_EXEC_NACK, NAME(MSG_SQL_EXEC_NACK)){};

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgSqlMixin::copy_data_on_response(request);
			  MsgResponse::copy_data_on_response(request);
		  }

	};

	class IW_SQLLITE_API MsgSqlStepReq: 
		public MsgSqlMixin, public MsgRequest
	{
	public:
		MsgSqlStepReq():
		  MsgRequest(MSG_SQL_STEP_REQ, NAME(MSG_SQL_STEP_REQ)){};

		sqlite3_stmt *pStmt;

	};

	class IW_SQLLITE_API MsgSqlStepAck: 
		public MsgSqlMixin, public MsgResponse
	{
	public:
		MsgSqlStepAck():
		  MsgResponse(MSG_SQL_STEP_ACK, NAME(MSG_SQL_STEP_ACK)){};

		int rc;

		virtual void copy_data_on_response(IN IwMessage *request)
		{
			MsgSqlMixin::copy_data_on_response(request);
			MsgResponse::copy_data_on_response(request);
		}

	};

	class IW_SQLLITE_API MsgSqlStepNack: 
		public MsgSqlMixin, public MsgResponse
	{
	public:
		MsgSqlStepNack():
		  MsgResponse(MSG_SQL_STEP_NACK, NAME(MSG_SQL_STEP_NACK)){};

		  virtual void copy_data_on_response(IN IwMessage *request)
		  {
			  MsgSqlMixin::copy_data_on_response(request);
			  MsgResponse::copy_data_on_response(request);
		  }

	};

	class IW_SQLLITE_API MsgSqlFinalizeReq: 
		public MsgSqlMixin, public MsgRequest
	{
	public:
		MsgSqlFinalizeReq():
		  MsgRequest(MSG_SQL_FINALIZE_REQ, NAME(MSG_SQL_FINALIZE_REQ)){};

		  sqlite3_stmt *pStmt;

	};

	/**
	Sqlite fiber non blocking connectivity
	**/
	class IW_SQLLITE_API SqliteSession
	{
	public:
		SqliteSession(void);

		virtual ~SqliteSession(void);

		ApiErrorCode sqlite3_open(const char *connection_url, int &rc);

		ApiErrorCode sqlite3_prepare( 
			const char *zSql,       /* SQL statement, UTF-8 encoded */
			int nByte,              /* Maximum length of zSql in bytes. */
			sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
			const char **pzTail,    /* OUT: Pointer to unused portion of zSql */
			int &res				/* OUT: sqlite result */
			);

		const char *sqlite3_errmsg();

		void sqlite3_close();

		ApiErrorCode sqlite3_exec( 
			const char *sql,                           /* SQL to be evaluated */
			int (*callback)(void*,int,char**,char**),  /* Callback function */
			void *,                                    /* 1st argument to callback */
			char **errmsg,                             /* Error msg written here */
			int &rc									   /* OUT: sqlite result */	
			);

		ApiErrorCode sqlite3_finalize(
			sqlite3_stmt *pStmt,
			int &rc						   
			);

		ApiErrorCode sqlite3_step( 
			sqlite3_stmt *pStmt,
			int &rc						   
			);

		ApiErrorCode sqlite3_reset( 
			sqlite3_stmt *pStmt,
			int &rc						   
			);

		int sqlite3_busy_timeout(int ms);

		sqlite3_int64 sqlite3_last_insert_rowid();

		int sqlite3_changes();

	private:

		int _sessionId;

		sqlite3 *_db;

	};


}


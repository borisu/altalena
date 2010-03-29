using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.SQLite;
using System.Data;

namespace iw_voicedb
{
    public class MemoTable : SqlObject
    {

        public class Memo
        {
            public string Username;

            public string Key;

            public string Value;
        }

        SQLiteConnection _conn;

        public MemoTable(string connectionString)
        {
            _conn = new SQLiteConnection(connectionString);
        }

        public MemoTable()
        {

        }

        public MemoTable(SQLiteConnection conn)
        {
            _conn = conn;
        }
        
        public void Create(SQLiteConnection conn)
        {
            if (conn == null)
            {
                throw new DataBaseException("SQL connection is NULL");
            }

            ExecuteNonQuery(conn,
                 @"DROP TABLE [Memos]");

            ExecuteNonQuery(conn,
                 @"CREATE TABLE [Memos] (
                    [MemoId] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL ,
                    [Username] varchar(255) collate BINARY NOT NULL default '',
                    [Key] varchar(50) NOT NULL,
                    [Value] varchar(50) NOT NULL)");

        }

        public int InsertMemo(SQLiteConnection conn, string userName, string key, string value)
        {
            if (conn == null)
            {
                throw new DataBaseException("SQL connection is NULL");
            }

            
            Validate(key);
            Validate(value);
           
            int res = ExecuteNonQuery(conn,
                @"INSERT INTO Memos (Username,Key,Value) 
                 VALUES ('" + userName + "', " + AddQuotes(key) + ", " + AddQuotes(value) + ")");

            return res;
        }


        public Dictionary<string, Memo> GetMemosOfUser(string loginName)
        {
            Dictionary<string, Memo> 
                retSet = new Dictionary<string, Memo>();


            DataTable dt = ExecuteQuery(_conn,
                "SELECT * FROM MEMOS WHERE Username='" + loginName + "'");

            foreach (DataRow row in dt.Rows)
            {
                Memo memo = new Memo();
                memo.Username   = (string)row["Username"];
                memo.Key        = (string)row["Key"];
                memo.Value      = (string)row["Value"];

                retSet[memo.Key] = memo;

            }

            return retSet;
        }
    }
}

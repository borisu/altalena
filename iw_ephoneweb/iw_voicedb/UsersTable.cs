using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.SQLite;
using System.Data;

namespace iw_voicedb
{
    public class UsersTable : SqlObject, IDisposable
    {

        private SQLiteConnection _conn;

        public UsersTable(string connectionString)
        {
            _conn = new SQLiteConnection(connectionString);
        }

        public UsersTable(SQLiteConnection conn)
        {
            _conn = conn;
        }

        public UsersTable()
        {
         
        }

        public void Create()
        {
            Create(_conn);
        }


        public void Create(SQLiteConnection conn)
        {
            if (conn == null)
            {
                throw new DataBaseException("SQL connection is NULL");
            }

            ExecuteNonQuery(conn,
                 @"DROP TABLE IF EXISTS [Users]");

            ExecuteNonQuery(conn,
                 @"CREATE TABLE 'users' (
                      'PKID' varchar(36) collate BINARY NOT NULL default '',
                      'Username' varchar(255) collate BINARY NOT NULL default '',
                      'ApplicationName' varchar(100) 
                                        collate BINARY NOT NULL default '',
                      'Email' varchar(100) collate BINARY NOT NULL default '',
                      'Comment' varchar(255) collate BINARY default NULL,
                      'Password' varchar(128) collate BINARY NOT NULL default '',
                      'PasswordQuestion' varchar(255) collate BINARY default NULL,
                      'PasswordAnswer' varchar(255) collate BINARY default NULL,
                      'IsApproved' tinyint(1) default NULL,
                      'LastActivityDate' datetime default NULL,
                      'LastLoginDate' datetime default NULL,
                      'LastPasswordChangedDate' datetime default NULL,
                      'CreationDate' datetime default NULL,
                      'IsOnLine' tinyint(1) default NULL,
                      'IsLockedOut' tinyint(1) default NULL,
                      'LastLockedOutDate' datetime default NULL,
                      'FailedPasswordAttemptCount' int(11) default NULL,
                      'FailedPasswordAttemptWindowStart' datetime default NULL,
                      'FailedPasswordAnswerAttemptCount' int(11) default NULL,
                      'FailedPasswordAnswerAttemptWindowStart' datetime default NULL,
                      PRIMARY KEY  ('PKID')
                    )");

        }

        public int CreateUser(
                SQLiteConnection conn,
                string username,
                string password,
                string email,
                string passwordQuestion,
                string passwordAnswer,
                bool isApproved,
                object providerUserKey,
                string pApplicationName,
                DateTime createDate)
        {


            SQLiteCommand cmd = new SQLiteCommand("INSERT INTO `users`" +
                      " (PKID, Username, Password, Email, PasswordQuestion, " +
                      " PasswordAnswer, IsApproved," +
                      " Comment, CreationDate, LastPasswordChangedDate, LastActivityDate," +
                      " ApplicationName, IsLockedOut, LastLockedOutDate," +
                      " FailedPasswordAttemptCount, FailedPasswordAttemptWindowStart, " +
                      " FailedPasswordAnswerAttemptCount, FailedPasswordAnswerAttemptWindowStart)" +
                      " Values($PKID, $Username, $Password, $Email, $PasswordQuestion, " +
                      " $PasswordAnswer, $IsApproved, $Comment, $CreationDate, $LastPasswordChangedDate, " +
                      " $LastActivityDate, $ApplicationName, $IsLockedOut, $LastLockedOutDate, " +
                      " $FailedPasswordAttemptCount, $FailedPasswordAttemptWindowStart, " +
                      " $FailedPasswordAnswerAttemptCount, $FailedPasswordAnswerAttemptWindowStart)", conn);

            cmd.Parameters.Add("$PKID", DbType.String).Value = providerUserKey.ToString();
            cmd.Parameters.Add("$Username", DbType.String, 255).Value = username;
            cmd.Parameters.Add("$Password", DbType.String, 255).Value = password;
            cmd.Parameters.Add("$Email", DbType.String, 128).Value = email;
            cmd.Parameters.Add("$PasswordQuestion", DbType.String, 255).Value = passwordQuestion;
            cmd.Parameters.Add("$PasswordAnswer", DbType.String, 255).Value = passwordAnswer;
            cmd.Parameters.Add("$IsApproved", DbType.Boolean).Value = isApproved;
            cmd.Parameters.Add("$Comment", DbType.String, 255).Value = "";
            cmd.Parameters.Add("$CreationDate", DbType.DateTime).Value = createDate;
            cmd.Parameters.Add("$LastPasswordChangedDate", DbType.DateTime).Value = createDate;
            cmd.Parameters.Add("$LastActivityDate", DbType.DateTime).Value = createDate;
            cmd.Parameters.Add("$ApplicationName", DbType.String, 255).Value = pApplicationName;
            cmd.Parameters.Add("$IsLockedOut", DbType.Boolean).Value = false;
            cmd.Parameters.Add("$LastLockedOutDate", DbType.DateTime).Value = createDate;
            cmd.Parameters.Add("$FailedPasswordAttemptCount", DbType.Int32).Value = 0;
            cmd.Parameters.Add("$FailedPasswordAttemptWindowStart", DbType.DateTime).Value = createDate;
            cmd.Parameters.Add("$FailedPasswordAnswerAttemptCount", DbType.Int32).Value = 0;
            cmd.Parameters.Add("$FailedPasswordAnswerAttemptWindowStart", DbType.DateTime).Value = createDate;


            int recAdded = ExecuteNonQuery(cmd);

            cmd.Dispose();

            return recAdded;


        }

        public int CreateUser(
                string username,
                string password,
                string email,
                string passwordQuestion,
                string passwordAnswer,
                bool isApproved,
                object providerUserKey,
                string pApplicationName,
                DateTime createDate)
        {
            return CreateUser(_conn,
                username,
                password,
                email,
                passwordQuestion,
                passwordAnswer,
                isApproved,
                providerUserKey,
                pApplicationName,
                createDate);

        }

        public void Dispose()
        {
            if (_conn != null)
            {
                _conn.Dispose();
                _conn.Close();
            }
        }
    }
}

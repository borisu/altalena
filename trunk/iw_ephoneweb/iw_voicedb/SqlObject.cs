using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.SQLite;
using System.Data;

namespace iw_voicedb
{
    public class SqlObject
    {
        public string AddQuotes(string value)
        {
            return "\"" + value + "\"";
        }

        public void Validate(string arg)
        {
            if (arg == null ||
                arg.Contains("\"") ||
                arg.Contains("\'"))
            {
                throw new TypeValidationException("argument cannot be null");
            }
        }

        public DataTable ExecuteQuery(SQLiteConnection conn, string commandText)
        {
         
            DataTable dt = new DataTable();

            SQLiteCommand cmd = new SQLiteCommand(conn);
            cmd.CommandText = commandText;

            SQLiteDataReader reader = cmd.ExecuteReader();

            dt.Load(reader);


            reader.Close();
            cmd.Dispose();
            

            return dt;

        }

        public int ExecuteNonQuery(SQLiteCommand cmd)
        {

            int recAdded = cmd.ExecuteNonQuery();

            return recAdded;

        }

        public int ExecuteNonQuery(SQLiteConnection conn, string commandText)
        {
            SQLiteCommand cmd = new SQLiteCommand(conn);

            cmd.CommandText = commandText;

            int recAdded = cmd.ExecuteNonQuery();
            cmd.Dispose();

            return recAdded;


        }
    }
}

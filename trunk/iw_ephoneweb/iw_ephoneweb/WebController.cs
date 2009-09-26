using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Data.SQLite;
using iw_voicedb;
using System.Web.UI.WebControls;

namespace iw_ephoneweb
{
    public class WebController
    {
        static SQLiteConnection GetDatabaseConnection()
        {
            SQLiteConnection conn =
                (SQLiteConnection)HttpContext.Current.Session[SessionConstants.Connection];

            if (conn == null)
            {
                throw new Exception("Cannot establish database conenc");
            }

            return conn;
        }

        static string GetLoggedInUsername()
        {
            string loggedInUser = HttpContext.Current.User.Identity.Name;
            if (loggedInUser == null)
            {
                throw new Exception("No logged in user found");
            };

            return loggedInUser;

        }

        public static void InsertMemo(
            System.Web.UI.WebControls.TextBox keyTextBox,
            System.Web.UI.WebControls.TextBox valueTextBox)
        {
            SQLiteConnection conn = GetDatabaseConnection();
            string loggedInUser = GetLoggedInUsername();

            MemoTable mt = new MemoTable();
            mt.InsertMemo(conn, loggedInUser,keyTextBox.Text, valueTextBox.Text);

        }

        public static void InitializeMemoTable(System.Web.UI.WebControls.Table tableControl)
        {

            SQLiteConnection conn = GetDatabaseConnection();
            string loggedInUser = GetLoggedInUsername();
            


            MemoTable mt = new MemoTable(conn);
            Dictionary<string, MemoTable.Memo> memos = 
                mt.GetMemosOfUser(loggedInUser);

            foreach (MemoTable.Memo memo in memos.Values)
            {
                
                TableRow  raw = new TableRow();

                // check box
                TableCell checkBoxCell = 
                    new TableCell();
                checkBoxCell.Controls.Add(new CheckBox());

                raw.Cells.Add(checkBoxCell);

                // key
                TableCell keyCell = 
                    new TableCell();
                keyCell.Text = memo.Key;
                raw.Cells.Add(keyCell);

                // value 
                TextBox tb = new TextBox();
                tb.Text = memo.Value;
                TableCell valueCell = 
                    new TableCell();
                valueCell.Controls.Add(tb); 
                raw.Cells.Add(valueCell);

                // update button
                TableCell updateButtonCell = 
                    new TableCell();
                Button updateBtn = new Button();
                updateBtn.Text = "Update";
                updateButtonCell.Controls.Add(updateBtn);
                raw.Cells.Add(updateButtonCell);

                // update button
                TableCell deleteButtonCell =
                    new TableCell();
                Button deleteBtn = new Button();
                deleteBtn.Text = "Delete";
                updateButtonCell.Controls.Add(deleteBtn);
                raw.Cells.Add(updateButtonCell);

                tableControl.Rows.Add(raw);
            }

        }
    }
}

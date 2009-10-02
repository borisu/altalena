using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.Security;
using iw_ephoneweb.DataSetTableAdapters;
using System.Data;

namespace iw_ephoneweb
{
    public partial class _Default : System.Web.UI.Page
    {

        protected void DeleteButton_Click(object sender, EventArgs e)
        {

            Table tableCtrl = LoginViewControl.FindControl("MemosTableCtrl") as Table;

            // restore checked boxes
            bool header = true;
            foreach (TableRow row in tableCtrl.Rows)
            {
                if (header)
                {
                    header = false;
                    continue;
                }
                CheckBox checkBox = row.Cells[0].Controls[0] as CheckBox;

                if (checkBox.Checked)
                {
                    MemosTableAdapter memosAdapter
                         = new MemosTableAdapter();
                    int memoId = GetMemoId(checkBox.ID);

                    memosAdapter.DeleteMemo(memoId);

                }
          
                
            }

        }
        
        protected void  ShowAllCheckBox_CheckedChanged(object sender, EventArgs e)
        {

           Table tableCtrl = LoginViewControl.FindControl("MemosTableCtrl") as Table;

           foreach (TableRow row in tableCtrl.Rows)
           {
               CheckBox checkBox = row.Cells[0].Controls[0] as CheckBox;
               checkBox.Checked = (sender as CheckBox).Checked;
           }
            
        }

        static int GetMemoId(string textID)
        {
            string[] substrs = textID.Split('_');
            int memoIdInt = int.Parse(substrs[1]);
            return memoIdInt;
        }

        static public void KeyTextChanged(object sender, EventArgs args)
        {
            TextBox textBox = sender as TextBox;

            int memoId = GetMemoId(textBox.ID);

            MemosTableAdapter memosAdapter
              = new MemosTableAdapter();

            int result = memosAdapter.UpdateKey(textBox.Text,memoId);

        }

        static public void ValueTextChanged(object sender, EventArgs args)
        {
            TextBox textBox = sender as TextBox;

            int memoId = GetMemoId(textBox.ID);

            MemosTableAdapter memosAdapter
              = new MemosTableAdapter();

            int result = memosAdapter.UpdateValue(textBox.Text, memoId);

        }

        protected void InitMemosTable(Table tableCtrl)
        {
                     
            string loggedInUser = HttpContext.Current.User.Identity.Name;

            TableRow headerRow = tableCtrl.Rows[0];

            tableCtrl.Controls.Clear();
            tableCtrl.Rows.Add(headerRow);
            

            MemosTableAdapter memosAdapter
              = new MemosTableAdapter();

            DataSet.MemosDataTable queryResult =
              memosAdapter.GetMemosByUser(loggedInUser);

            foreach (DataRow memosRow in queryResult.Rows)
            {

                long memoId = (long)memosRow["MemoId"];
                string memoKey = (string)memosRow["Key"];
                string memoValue = (string)memosRow["Key"];

                TableRow tr = new TableRow();

                TableCell checkBoxCell = new TableCell();
                CheckBox checkBox = new CheckBox();
                checkBox.ID = "checkbox_" + memoId;
                checkBoxCell.Controls.Add(checkBox);
                tr.Cells.Add(checkBoxCell);

                TableCell keyTextBoxCell = new TableCell();
                TextBox keyTextBox = new TextBox();
                keyTextBox.ID = "keytextbox_" + memoId;
                keyTextBox.AutoPostBack = true;



                keyTextBox.Text = memoKey + ":" + keyTextBox.ID;
                keyTextBox.TextChanged += KeyTextChanged;



                keyTextBoxCell.Controls.Add(keyTextBox);
                tr.Cells.Add(keyTextBoxCell);

                TableCell valueTextBoxCell = new TableCell();
                TextBox valueTextBox = new TextBox();
                valueTextBox.ID = "valuetextbox_" + memoId;
                valueTextBox.AutoPostBack = true;



                valueTextBox.Text = memoValue + ":" + valueTextBox.ID;
                valueTextBox.TextChanged += ValueTextChanged;



                valueTextBoxCell.Controls.Add(valueTextBox);
                tr.Cells.Add(valueTextBoxCell);


                tableCtrl.Rows.Add(tr);

            }
        }

        protected void MemosTableCtrl_Init(object sender, EventArgs e)
        {
            Table tableCtrl = sender as Table;
            InitMemosTable(tableCtrl);
        }

        protected void Page_Load(object sender, EventArgs e)
        {

            
          
        }

        protected void MemosTableCtrl_PreRender(object sender, EventArgs e)
        {
            Dictionary<string, bool> checkedItems = new Dictionary<string,bool>();

            Table tableCtrl = sender as Table;

            // remember all checked boxes
            
            foreach (TableRow row in tableCtrl.Rows)
            {
                CheckBox checkBox = row.Cells[0].Controls[0] as CheckBox;
                checkedItems[checkBox.ID] = checkBox.Checked;
            }

            // synchronize table with db
            InitMemosTable(tableCtrl);

            // restore checked boxes
            bool header = true;
            foreach (TableRow row in tableCtrl.Rows)
            {
                if (header)
                {
                    header = false;
                    continue;
                }
                CheckBox checkBox = row.Cells[0].Controls[0] as CheckBox;
                if (checkedItems.ContainsKey(checkBox.ID))
                {
                    checkBox.Checked = checkedItems[checkBox.ID];
                }
                
            }

        }


        protected void InsertMemoButton_Click(object sender, EventArgs e)
        {
            TextBox keyTextBox = 
                (TextBox)LoginViewControl.FindControl("MemoKeyTextBox");

            TextBox valueTextBox =
                (TextBox)LoginViewControl.FindControl("MemoValueTextBox");

            string loggedInUser = HttpContext.Current.User.Identity.Name;

            MemosTableAdapter memosAdapter
              = new MemosTableAdapter();

            int result = 
                memosAdapter.InsertMemo(loggedInUser, keyTextBox.Text, valueTextBox.Text);

            keyTextBox.Text = "";
            valueTextBox.Text = "";

         }

      
        
       
            

    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Web.Security;

namespace iw_ephoneweb
{
    public partial class _Default : System.Web.UI.Page
    {

        protected void Page_Load(object sender, EventArgs e)
        {

        }

        protected void InsertMemoButton_Click(object sender, EventArgs e)
        {
            TextBox keyTextBox = 
                (TextBox)LoginViewControl.FindControl("MemoKeyTextBox");

            TextBox valueTextBox =
                (TextBox)LoginViewControl.FindControl("MemoValueTextBox");

            WebController.InsertMemo(keyTextBox, valueTextBox);

        }

        protected void MemosTable_Load(object sender, EventArgs e)
        {
            WebController.InitializeMemoTable((Table)sender);
        }
        
    }
}

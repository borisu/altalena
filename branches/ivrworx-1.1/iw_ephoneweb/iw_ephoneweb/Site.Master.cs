using System;
using System.Collections;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Xml.Linq;

namespace iw_ephoneweb
{
    public partial class Site : System.Web.UI.MasterPage
    {
        protected void Page_Load(object sender, EventArgs e)
        {

        }

        protected void SiteLogin_LoggedIn(object sender, EventArgs e)
        {
            MembershipUser membershipUser = Membership.GetUser();
            if (membershipUser != null)
            {
                string loggedinuser = Membership.GetUser().ToString();

                Login loginCtrl = (Login)sender;
                

            }
        }
    }
}

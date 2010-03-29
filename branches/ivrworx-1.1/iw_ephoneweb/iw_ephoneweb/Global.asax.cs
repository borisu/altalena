using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Security;
using System.Web.SessionState;
using System.Data.SQLite;
using System.Configuration;


namespace iw_ephoneweb
{
    public class Global : System.Web.HttpApplication
    {

        protected void Application_Start(object sender, EventArgs e)
        {

        }

        protected void Session_Start(object sender, EventArgs e)
        {
            System.Configuration.Configuration rootWebConfig =
               System.Web.Configuration.WebConfigurationManager.OpenWebConfiguration("/");

            System.Configuration.ConnectionStringSettings connString = null;
            if (0 < rootWebConfig.ConnectionStrings.ConnectionStrings.Count)
            {
                connString =
                    rootWebConfig.ConnectionStrings.ConnectionStrings["ConnString"];
            }

            if (connString == null)
            {
                return;
            }

            // open db connection per session
            SQLiteConnection conn = new SQLiteConnection();

            conn.ConnectionString = connString.ConnectionString;

            conn.Open();

            Session[SessionConstants.Connection] = conn;


        }

        protected void Application_BeginRequest(object sender, EventArgs e)
        {

        }

        protected void Application_AuthenticateRequest(object sender, EventArgs e)
        {

        }

        protected void Application_Error(object sender, EventArgs e)
        {

        }

        protected void Session_End(object sender, EventArgs e)
        {

            SQLiteConnection conn = (SQLiteConnection)Session[SessionConstants.Connection];

            if (conn != null)
            {
                conn.Dispose();
                conn.Close();
                Session[SessionConstants.Connection] = null;

            }

        }

        protected void Application_End(object sender, EventArgs e)
        {

        }
    }
}
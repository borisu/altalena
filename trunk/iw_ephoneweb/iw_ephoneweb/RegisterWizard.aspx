<%@ Page Language="C#" AutoEventWireup="false" CodeBehind="RegisterWizard.aspx.cs" Inherits="iw_ephoneweb.RegisterWizard" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server" >
    <title>Polly Registration</title>
    <link href="style.css" rel="stylesheet" type="text/css" />
</head>
<body class="register-body">
    <form id="RegisterForm" runat="server">
    
        <!------------>
        <!-- HEADER -->
        <!------------>
        <div id="create-user-header">
            <table id="register-header-table">
             <tr>
              <td><img src="images/polly.png" alt="parrot logo" align="left"/></td>
              <td><h1>Sign-up to <span style="font-weight:bold; color:#73868C;">Polly</span> application.</h1></td>
             </tr>
            </table>
        </div>
    
        <!------------------------->
        <!-- REGISTRATION WIZARD -->
        <!------------------------->
        <div id="create-user-body">
            <asp:CreateUserWizard ID="CreateUserWizard" runat="server" 
                CancelDestinationPageUrl="~/Default.aspx" DisplayCancelButton="True">
                <WizardSteps>
                    <asp:CreateUserWizardStep ID="CreateUserWizardStep" runat="server" EnableViewState="True">
                    </asp:CreateUserWizardStep>
                    <asp:CompleteWizardStep ID="CompleteWizardStep" runat="server">
                    </asp:CompleteWizardStep>
                </WizardSteps>
            </asp:CreateUserWizard>
        </div>
        
    </form>
</body>
</html>

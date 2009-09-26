<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Default.aspx.cs" MasterPageFile="~/Site.Master" Inherits="iw_ephoneweb._Default" %>

<asp:Content ID="Content1" ContentPlaceHolderID="ContentPlaceHolder1" runat="server">
    <asp:LoginView ID="LoginViewControl" runat="server">
            <AnonymousTemplate>
                <div id="anonymous-center">
                    <h1>Welcome to <span style="font-weight:bold; color:#C4DA64;">Polly the Parrot</span> Application</h1>
                    <p>
                        <!--<img class="imgleft" src="images/info.png" alt="" />-->
                        <strong> 
                        <em>
                        This simple application enables you to receive via phone simple pieces of information when internet connection is not available. 
                        </em>
                        </strong>
                        <br/>
                    </p>
                   <br />
                   <p>
                    <h1>How it works?</h1>
                   <table>
                    <tr>
                      <td><h1>1.</h1>Login to the site and insert short info and its key, that you would like to be voiced over phone</td>
                      <td><h1>2.</h1>Call the +1-54-12345 number, and authenticate.</td>
                      <td><h1>3.</h1>Press DTMF sequence which corresponds to key of the info, and listen to your info being voiced.</td>
                    </tr>
                   </table>
                       <p>
                           IVR application that powers up Polly is built upon
                           <a href="http://code.google.com/p/altalena/">ivrworx</a> infrastructure.</p>
                       <p>
                       </p>
                       <p>
                       </p>
                       </p>
                   </p>
                </div>
                <div id="anonymous-right">
                </div>
            </AnonymousTemplate> 
           
            <LoggedInTemplate>
                <p>
                    Select <a>All</a>, <a>None</a>.
                    <asp:Button ID="DeleteButton" runat="server" Text="Delete Selected" />
                </p>
                <p>
               
                    <asp:Label ID="MessageLabel" runat="server" Text=""></asp:Label>
                    <table ID="UpdateMemoTable"> 
                        <tr>
                            <td>Memo Id</td>
                            <td>Value</td>
                            <td>Value</td>
                        </tr>
                        <tr>  
                            <td><asp:TextBox ID="MemoKeyTextBox" runat="server"></asp:TextBox></td>
                            <td><asp:TextBox ID="MemoValueTextBox" runat="server"></asp:TextBox></td>
                            <td><asp:Button ID="InsertMemoButton" runat="server" Text="Button" onclick="InsertMemoButton_Click"/></td>
                        </tr>
                    </table>
                </p> 
                <hr/>
                
                <p>
                  <asp:Table ID="MemosTable" runat="server" onload="MemosTable_Load">
                  </asp:Table>
                </p>
            </LoggedInTemplate>
       </asp:LoginView>
</asp:Content>
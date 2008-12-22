using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Office.Interop.Visio;
using System.Runtime.InteropServices;
using OleCommandTarget;

namespace callfloweditor
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            AppInit();
        }

        public Microsoft.Office.Interop.Visio.Application GetApplication()
        {
            return visioControl.Document.Application;
        }

        public Document GetDocument()
        {
            return visioControl.Document;
        }

        public void AppInit()
        {

            GetDocument().PaperSize = VisPaperSizes.visPaperSizeA4;

            string currDirectory = System.Environment.CurrentDirectory;

            Document altStencil =
               GetApplication().Documents.OpenEx(
               currDirectory + @"\Altalena.vss",
               (short)VisOpenSaveArgs.visOpenDocked);

        }



        private void SendCommand(VisUICmds commandID)
        {
            IOleCommandTarget commandTarget =
                (IOleCommandTarget)visioControl.GetOcx();

            try
            {
                Guid CLSID_Application =
                    new Guid("00021A20-0000-0000-C000-000000000046");

                commandTarget.Exec(ref CLSID_Application,
                    (UInt32)commandID, 0, null, null);
            }
            catch (System.Runtime.InteropServices.COMException ex)
            {
                MessageBox.Show(ex.ToString());
            }

        }

        private void ConnectionButton_Click(object sender, EventArgs e)
        {
            SendCommand(VisUICmds.visCmdDRConnectorTool);
        }

        private void validateButton_Click(object sender, EventArgs e)
        {
            
            VisioDiagramValidator validator = new VisioDiagramValidator(GetDocument());

            validator.validate();


        }



    }
}
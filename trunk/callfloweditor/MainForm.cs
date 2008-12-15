using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Office.Interop.Visio;

namespace callfloweditor
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            AppInit();
        }

        public void AppInit()
        {
            _application = visioControl.Document.Application;

            string currDirectory = System.Environment.CurrentDirectory;

            Document altStencil = 
               _application.Documents.OpenEx(
               currDirectory + @"\Altalena.vss", 
               (short)VisOpenSaveArgs.visOpenDocked);
        }

        private Microsoft.Office.Interop.Visio.Application _application;
    }
}
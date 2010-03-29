using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Office.Interop.Visio;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.IO;


namespace callfloweditor
{
    class LuaScriptGenerator
    {
        private Microsoft.Office.Interop.Visio.Application _visApplication;

        Microsoft.Office.Interop.Visio.Document _visDocument;

        static string _currScriptFileName = "Untitled.lua";

        public LuaScriptGenerator(Microsoft.Office.Interop.Visio.Document visDocument)
        {
            _visDocument = visDocument;
            _visApplication = visDocument.Application;
        }

        public bool generate()
        {

            Page page = _visDocument.Pages[1];

            StringBuilder builder = new StringBuilder();

            builder.AppendLine("-- FUNCTIONS DEFS START --");
            foreach (Shape visShape in page.Shapes)
            {

                CallFlowShape callFlowshape = CallFlowShapeFactory.CreateInstance(visShape);
                if (callFlowshape == null)
                {
                    continue;
                }

                string generated_block = callFlowshape.GenerateFunctionBlock();
                if (generated_block == null)
                {
                    continue;
                }


                builder.AppendLine("                          ");
                builder.AppendLine(generated_block);
                builder.AppendLine("                          ");

            }// foreach 


            // generate main block
            builder.AppendLine("-- FUNCTIONS DEFS END --  ");
            builder.AppendLine("                          ");
            builder.AppendLine("                          ");
            builder.AppendLine("-- MAIN BLOCK START --    ");
            builder.AppendLine("nextfunc = start();       ");
            builder.AppendLine("while (nextfunc ~= nil) do");
            builder.AppendLine("   nextfunc = nextfunc(); ");
            builder.AppendLine("end;                      ");
            builder.AppendLine("-- MAIN BLOCK END --      ");

            string script = builder.ToString();

            //MessageBox.Show(script);


            SaveFileDialog saveFileDialog = new SaveFileDialog();
            DialogResult result = DialogResult.No;

            try
            {
                
                

                // Set up the save file dialog and let the user specify the
                // name to save the document to.
                saveFileDialog = new SaveFileDialog();
                saveFileDialog.Title = "Save Script File As";
                saveFileDialog.Filter = "Lua files (*.lua)|*.lua|All files (*.*)|*.*";
                saveFileDialog.FilterIndex = 1;
                saveFileDialog.InitialDirectory = System.Windows.Forms.Application.StartupPath;
                saveFileDialog.FileName = _currScriptFileName;

                result = saveFileDialog.ShowDialog();

                if (result == DialogResult.OK)
                {
                    _currScriptFileName  = saveFileDialog.FileName;
                }
                else
                {
                    return false;
                }

                
            }
            finally
            {
                // Make sure the dialog is cleaned up.
                if (saveFileDialog != null)
                {
                    saveFileDialog.Dispose();
                }
            }

            // save generated script to file
            TextWriter tw = new StreamWriter(_currScriptFileName);
            tw.WriteLine(script);
            tw.Close();
            
            return true;

        }// validate
    }
}

using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.Office.Interop.Visio;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace callfloweditor
{

    class VisioDiagramValidator
    {

        private Microsoft.Office.Interop.Visio.Application _visApplication;

        Microsoft.Office.Interop.Visio.Document _visDocument;

        public VisioDiagramValidator(Microsoft.Office.Interop.Visio.Document visDocument)
        {
            _visDocument = visDocument;
            _visApplication = visDocument.Application;
        }

        public bool validate(bool suppressGuiForSuccess)
        {

            foreach (Page page in _visDocument.Pages)
            {
                System.Console.WriteLine("Page:" + page.NameU);
                foreach (Shape shape in page.Shapes)
                {
                    System.Console.WriteLine("\tShape:" + shape.NameU);

                    foreach (Connect vsoConnect in shape.FromConnects)
                    {

                        System.Console.WriteLine("\t\tFromConnects:Connection:" + vsoConnect.Index);
                        System.Console.WriteLine("\t\t\tFromConnects:ToSheet:" + vsoConnect.ToSheet.NameU);
                        System.Console.WriteLine("\t\t\tFromConnects:FromSheet:" + vsoConnect.FromSheet.NameU);
                        System.Console.WriteLine("\t\t\tFromConnects:FromPart:" + vsoConnect.FromPart);
                    }

                    foreach (Connect vsoConnect in shape.Connects)
                    {

                        System.Console.WriteLine("\t\tConnects:Connection:" + vsoConnect.Index);
                        System.Console.WriteLine("\t\t\tConnects:ToSheet:" + vsoConnect.ToSheet.NameU);
                        System.Console.WriteLine("\t\t\tConnects:FromSheet:" + vsoConnect.FromSheet.NameU);
                        System.Console.WriteLine("\t\t\tConnects:FromPart:" + vsoConnect.FromPart);
                    }

                }
            }


            if (true) return true;


            // check that there are no invalid connectors (not connected to anything)
            foreach (Page page in _visDocument.Pages)
            {
                int start_count = 0;

                foreach (Shape shape in page.Shapes)
                {

                    try
                    {
                        CallFlowShapeFactory.CreateInstance(shape);
                    }
                    catch (System.Exception e)
                    {

                        MessageBox.Show(
                                       "Validation failed on page " + page.Name + ", for shape " + shape.NameU + " - " + e.Message + ".",
                                       "Validation Error",
                                       MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return false;
                    }


                    if (shape.Master.NameU.Equals(CallFlowShape.Start_MasterName))
                    {

                        start_count++;

                        if (start_count > 1)
                        {
                            MessageBox.Show(
                                "Found multiple 'Start' object on the page " + page.Name + ".",
                                "Validation Error",
                                 MessageBoxButtons.OK, MessageBoxIcon.Error);

                            return false;
                        }

                    }// if 


                }// foreach 

                if (start_count == 0)
                {

                    MessageBox.Show(
                      "Cannot find 'Start' object on the page " + page.Name + ".",
                      "Validation Error",
                      MessageBoxButtons.OK, MessageBoxIcon.Error);

                    return false;
                }


            }// foreach

            if (!suppressGuiForSuccess)
            {
                MessageBox.Show(
                           "The document has been successfully validated",
                           "Validation Success",
                           MessageBoxButtons.OK, MessageBoxIcon.Information);
            }


            return true;

        }// validate

    }

}

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

        public void validate()
        {
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
                                        "Validation failed  on page " +  page.Name + ", for shape " + shape.NameU + " - " + e.Message + ".",
                                        "Validation Error",
                                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                        return;
                    }


                    if (shape.Master.NameU.Equals(CallFlowShape.Start_NameU))
                    {
                        
                        start_count++;

                        if (start_count > 1)
                        {
                            MessageBox.Show(
                                "Cannot Validate! Found multiple 'Start' object on the page " + page.Name + ".",
                                "Validation Error",
                                 MessageBoxButtons.OK, MessageBoxIcon.Error);

                            return;
                        }

                    }// if 

                     
                }// foreach 

                if (start_count == 0)
                {

                    MessageBox.Show(
                      "Cannot Validate! Cannot find 'Start' object on the page " + page.Name + ".",
                      "Validation Error",
                      MessageBoxButtons.OK, MessageBoxIcon.Error);

                    return;
                }


            }// foreach

            MessageBox.Show(
                           "The document has been successfully validated",
                           "Validation Success",
                           MessageBoxButtons.OK, MessageBoxIcon.Information);

        }// validate

    }
    
}

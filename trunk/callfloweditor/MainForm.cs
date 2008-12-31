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
        /// <summary>Visio stencil suffix VSS.</summary>
        public const string VssSuffix = ".VSS";
        /// <summary>Visio xml stencil suffix VSX.</summary>
        public const string VsxSuffix = ".VSX";
        /// <summary>Visio drawing suffix VSD.</summary>
        public const string VsdSuffix = ".VSD";
        /// <summary>Visio xml drawing suffix VDX.</summary>
        public const string VdxSuffix = ".VDX";


        /// <summary>The drawingControlList contains a list of names of
        /// documents open in the drawing control.</summary>
        private System.Collections.ArrayList drawingControlList =
            new System.Collections.ArrayList();

        /// <summary>The EventSink class handles Visio events.
        /// </summary>
        private AltalenaEventSink visioEventSink;

        [CLSCompliant(false)]
        public Microsoft.Office.Interop.Visio.Application GetApplication()
        {
            return visioControl.Document.Application;
        }

        [CLSCompliant(false)]
        public Microsoft.Office.Interop.Visio.Document GetDocument()
        {
            return visioControl.Document;
        }

        public MainForm()
        {
            InitializeComponent();
            visioControl.Src = "";
        }

        public void setUpVisioDrawing()
        {

            // Start the event sink.
            initializeEventSink();

            // Open the stencil with the product shapes.
            openProductStencil();

            
        }

        void initializeEventSink()
        {
            Document targetDocument;
            Microsoft.Office.Interop.Visio.Application targetApplication;

            try
            {

                // Release the previous event sink.
                visioEventSink = null;

                // Create an event sink to hook up events to the Visio
                // application and document.
                visioEventSink = new AltalenaEventSink();
                targetApplication = (Microsoft.Office.Interop.Visio.Application)
                    visioControl.Window.Application;

                targetDocument = (Document)visioControl.Document;

                visioEventSink.AddAdvise(targetApplication, targetDocument);

            }

            catch (COMException error)
            {
               throw;
            }
        }

        void openProductStencil()
        {
             string stencilPath = System.Windows.Forms.Application.StartupPath + "\\Altalena.vss";
 
             Document targetStencil = null;
 
             Documents targetDocuments;
             targetDocuments = (Documents)visioControl.Window.Application.Documents;
 
             targetStencil = targetDocuments.OpenEx(stencilPath,
                 (short)VisOpenSaveArgs.visOpenRO |
                 (short)VisOpenSaveArgs.visOpenNoWorkspace);
 
             // The drawing control is displaying a newly opened document.
             // Set the saved flag to true.
             visioControl.Document.Saved = true;
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

#region UI_Handlers

        private void validateButton_Click(object sender, EventArgs e)
        {
            DoValidateCmd();
        }

        private void newToolStripButton_Click(object sender, EventArgs e)
        {
            DoFileNew();
        }

        private void openToolStripButton_Click(object sender, EventArgs e)
        {
            DoFileOpenCmd(sender, e); 
        }

        private void openScriptDiagramToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoFileOpenCmd(sender, e); 
        }

        private void cutToolStripButton_Click(object sender, EventArgs e)
        {
            DoVisioCutCmd();
        }

        private void copyToolStripButton_Click(object sender, EventArgs e)
        {
            DoVisioCopyCmd();
        }

        private void pasteToolStripButton_Click(object sender, EventArgs e)
        {
            DoVisioPasteCmd();
        }

        private void helpToolStripButton_Click(object sender, EventArgs e)
        {
            DoShowAboutDlgCmd();
        }

        private void saveToolStripButton_Click(object sender, EventArgs e)
        {
            DoFileSaveCmd();
        }

        private void AltMainMenuStrip_VisibleChanged(object sender, EventArgs e)
        {
            DoVisibleChangedCmd();
        }


#endregion UI_Handlers

        void DoVisibleChangedCmd()
        {

            // If the drawing control is not on the list of initialized
            // drawing controls, the control needs to be initialized.
            if ((visioControl.Visible) &&
                (!drawingControlList.Contains(visioControl.Name)))
            {

                // Setup the drawing, open the product stencil and start the
                // event sink.
                setUpVisioDrawing();

                // Since all of the initialization tasks succeeded, save
                // the drawing control in the list of initialized controls.
                drawingControlList.Add(visioControl.Name);
            }

        }

        void DoFileNew()
        {
            SaveDrawing(true);

        }
        
        void DoValidateCmd()
        {
            VisioDiagramValidator validator = 
                new VisioDiagramValidator(GetDocument());

            validator.validate();
        }

        void DoShowAboutDlgCmd()
        {
            new AboutForm().Show(this);
        }

        void DoVisioCopyCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditCopy);
        }

        void DoVisioCutCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditCut);
        }

        void DoVisioPasteCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditPaste);
        }

        /// <summary>The onPrintDocumentPrintPage method handles the event that
        /// is raised when the end user hits Print in the print dialog.
        /// </summary>
        /// <param name="sender">The object that raised the event. This
        /// is ignored for this case.</param>
        /// <param name="e">The arguments associated with the
        /// event.</param>
        private void onPrintDocumentPrintPage(object sender,
            System.Drawing.Printing.PrintPageEventArgs e)
        {
            const int metafileOriginX = 0;
            const int metafileOriginY = 0;
            double pageWidth = 0;
            double pageHeight = 0;
            Shape temporaryShape;
            Page targetPage;

            targetPage = (Page)visioControl.Window.PageAsObj;

            // draw a  frame around the page so the bounding box of the
            // picture is the size of the page.
            pageWidth = targetPage.PageSheet.get_CellsSRC(
                (short)Microsoft.Office.Interop.Visio.VisSectionIndices.
                visSectionObject,
                (short)Microsoft.Office.Interop.Visio.VisRowIndices.
                visRowPage,
                (short)Microsoft.Office.Interop.Visio.VisCellIndices.
                visPageWidth).ResultIU;

            pageHeight = targetPage.PageSheet.get_CellsSRC(
                (short)Microsoft.Office.Interop.Visio.VisSectionIndices.
                visSectionObject,
                (short)Microsoft.Office.Interop.Visio.VisRowIndices.
                visRowPage,
                (short)Microsoft.Office.Interop.Visio.VisCellIndices.
                visPageHeight).ResultIU;

            temporaryShape = targetPage.DrawRectangle(0, 0,
                pageWidth, pageHeight);

            temporaryShape.get_CellsSRC(
                (short)Microsoft.Office.Interop.Visio.VisSectionIndices.
                visSectionObject,
                (short)Microsoft.Office.Interop.Visio.VisRowIndices.
                visRowFill,
                (short)Microsoft.Office.Interop.Visio.VisCellIndices.
                visFillPattern).ResultIU = 0;

            // Make the line white so it's not visible on the page.
            temporaryShape.get_CellsSRC(
                (short)Microsoft.Office.Interop.Visio.VisSectionIndices.
                visSectionObject,
                (short)Microsoft.Office.Interop.Visio.VisRowIndices.
                visRowLine,
                (short)Microsoft.Office.Interop.Visio.VisCellIndices.
                visLineColor).ResultIU = 1;

            // Create a metafile with the image of the page.
            System.Drawing.Imaging.Metafile printMetafile =
                new System.Drawing.Imaging.Metafile(
                (IntPtr)targetPage.Picture.Handle, false);

            temporaryShape.Delete();

            // Put the image into the graphics of the print document. This
            // causes the page image to be printed.
            e.Graphics.DrawImage(printMetafile,
                metafileOriginX, metafileOriginY);
        }

        /// <summary>The onMenuFilePrintPreviewClicked method handles the event
        /// that is raised when the user clicks on the Print Preview
        /// command in the menu. The System.Drawing.Printing objects are used
        /// to provide print preview to the user.</summary>
        /// <param name="sender">The object that raised the event. This
        /// is ignored for this case.</param>
        /// <param name="e">The arguments associated with the
        /// event. This is ignored for this case.</param>
        private void DoPrintPreviewCmd()
        {
            try
            {

                System.Drawing.Printing.PrintDocument printDocument = null;
                PrintPreviewDialog printPreviewDialog = null;

                // Create a print document to handle the print preview
                // functionality. This assumes the default settings.
                printDocument = new System.Drawing.Printing.PrintDocument();

                // Add the local print page handler to print document. This
                // allows the page image to be used as the printed graphics.
                printDocument.PrintPage +=
                    new System.Drawing.Printing.PrintPageEventHandler(
                    this.onPrintDocumentPrintPage);

                // Create a print preview dialog and show it to the user. The
                // handler will process the actual print.
                printPreviewDialog = new PrintPreviewDialog();
                printPreviewDialog.Document = printDocument;
                printPreviewDialog.ShowDialog();
            }
            catch (System.Drawing.Printing.InvalidPrinterException error)
            {
                System.Windows.Forms.MessageBox.Show(this, error.Message,
                    this.Text,
                    System.Windows.Forms.MessageBoxButtons.OK,
                    System.Windows.Forms.MessageBoxIcon.Exclamation);
            }
        }

        /// <summary>The onMenuFileSaveClicked method handles the event that is
        /// raised when the end user clicks on the File > Save command in the
        /// menu.</summary>
        /// <param name="sender">The object that raised the event. This
        /// is ignored for this case.</param>
        /// <param name="e">The arguments associated with the
        /// event. This is ignored for this case.</param>
        private void DoFileSaveCmd()
        {
            DialogResult result;

            // Prompt user to save changes.
            result = SaveDrawing(false);
        }

        /// <summary>The onMenuFileOpenClicked method handles the event that is
        /// raised when the user clicks on the File > Open command in the
        /// menu. Either a document or stencil can be opened.</summary>
        /// <param name="sender">The object that raised the event. This
        /// is ignored for this case.</param>
        /// <param name="e">The arguments associated with the
        /// event. This is ignored for this case.</param>
        private void DoFileOpenCmd(object sender, EventArgs e)
        {

            OpenFileDialog openFileDialog = null;
            Documents targetDocuments;
            string fileName;
            string fileNameForCompare;
            

            try
            {
                // Set up the open file dialog and let the user select the
                // file to open.
                openFileDialog = new OpenFileDialog();
                openFileDialog.Title = "Open Script Diagram";
                openFileDialog.Filter = "Drawing files (*.vsd)|*.vsd|All files (*.*)|*.*";
                openFileDialog.FilterIndex = 1;
                openFileDialog.InitialDirectory =
                    System.Windows.Forms.Application.StartupPath;

                if (openFileDialog.ShowDialog() == DialogResult.OK)
                {

                    // The user selected a valid file name and hit OK. Get the
                    // file name from the dialog and open the file.
                    fileName = openFileDialog.FileName;
                    fileNameForCompare = fileName.ToUpper(
                        System.Globalization.CultureInfo.CurrentCulture);
                    if ((fileNameForCompare.IndexOf(VssSuffix) > 0) ||
                        (fileNameForCompare.IndexOf(VsxSuffix) > 0))
                    {

                        targetDocuments =
                            (Documents)visioControl.Window.Application.Documents;

                        // Open the stencil read-only.
                        targetDocuments.OpenEx(fileName,
                            (short)VisOpenSaveArgs.visOpenRO +
                            (short)VisOpenSaveArgs.visOpenDocked);
                    }

                    else if ((fileNameForCompare.IndexOf(VsdSuffix) > 0) ||
                        (fileNameForCompare.IndexOf(VdxSuffix) > 0))
                    {

                        DialogResult result;

                        // Only one document can be open.
                        // Prompt user to save changes before closing the current
                        // document.
                        result = SaveDrawing(true);

                        if (DialogResult.Cancel != result)
                        {

                            // Open the new document.
                            GC.Collect();
                            GC.WaitForPendingFinalizers();

                            visioControl.Src = fileName;
                            visioControl.Document.Saved = true;

                            // Setup the drawing, open the product stencil, and
                            // start the event sink.
                            // Note that opening a different drawing will close
                            // the previous drawing and end the previous event sink.
                            setUpVisioDrawing();

                          
                        }
                    }
                }
            }

            finally
            {

                // Make sure the dialog is cleaned up.
                if (openFileDialog != null)
                {
                    openFileDialog.Dispose();
                }
            }
        }

        /// <summary>The SaveDrawing method prompts the user to save changes
        /// to the Visio document.</summary>
        /// <param name="drawingControl">Drawing control with the Visio
        ///  document to save.</param>
        /// <param name="promptFirst">Display "save changes" prompt.</param>
        /// <returns>The id of the message box button that dismissed
        /// the dialog.</returns>
        [CLSCompliant(false)]
        public DialogResult SaveDrawing(
            bool promptFirst)
        {

            if (visioControl == null)
            {
                return DialogResult.Cancel;
            }


            SaveFileDialog saveFileDialog = null;
            DialogResult result = DialogResult.No;
            string targetFilename = string.Empty;
            Document targetDocument;

            try
            {
                targetFilename = visioControl.Src;
                targetDocument = (Document)visioControl.Document;

                // Prompt to save changes.
                if (promptFirst == true)
                {

                    string prompt = string.Empty;
                    string title = string.Empty;

                    title ="Save Script File";

                    if (targetFilename == null)
                    {
                        return DialogResult.Cancel;
                    }

                    // Save changes to the existing drawing.
                    if (targetFilename.Length > 0)
                    {
                        prompt = "Save Script Diagram As";
                        prompt += Environment.NewLine;
                        prompt += targetFilename;
                    }
                    else
                    {

                        // Save changes as new drawing.
                        prompt = "Save Script Diagram As";
                    }
                    result = MessageBox.Show(prompt, title,
                        MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                }
                else
                {
                    result = DialogResult.Yes;
                }

                // Display a file browse dialog to select path and filename.
                if ((DialogResult.Yes == result) && (targetFilename.Length == 0))
                {

                    // Set up the save file dialog and let the user specify the
                    // name to save the document to.
                    saveFileDialog = new SaveFileDialog();
                    saveFileDialog.Title = "SaveDialogTitle";
                    saveFileDialog.Filter = "Drawing files (*.vsd)|*.vsd|All files (*.*)|*.*";
                    saveFileDialog.FilterIndex = 1;
                    saveFileDialog.InitialDirectory = System.Windows.Forms.Application.StartupPath;

                    if (saveFileDialog.ShowDialog() == DialogResult.OK)
                    {
                        targetFilename = saveFileDialog.FileName;
                    }
                }
                // Save the document to the filename specified by
                // the end user in the save file dialog, or the existing file name.
                if ((DialogResult.Yes == result) && (targetFilename.Length > 0))
                {

                    if (targetDocument != null)
                    {
                        targetDocument.SaveAs(targetFilename);
                    }

                    GC.Collect();
                    GC.WaitForPendingFinalizers();
                    visioControl.Src = targetFilename;
                    visioControl.Document.Saved = true;
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
            return result;
        }

       

        
       


    }
}
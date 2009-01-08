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

        public const string TitlePostfix = " - Altalena Call Flow Editor";

        public const string UntitledFileName = "Untitled";

        public string currFileNameInTitle = UntitledFileName;
        [CLSCompliant(false)]
        public string CurrFileNameInTitle
        {
            get { return visioControl.Src == "" ? UntitledFileName : System.IO.Path.GetFileName(visioControl.Src); }
        }

        const int visualFeedbackOn = 1;

        const int visualFeedbackOff = 0;

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
            Text = UntitledFileName + TitlePostfix;
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
            catch (COMException) 
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

        DialogResult SaveCheck()
        {
            if (visioControl.Document.Saved == true)
            {
                return DialogResult.Yes;
            }

            DialogResult result =
              MessageBox.Show("The diagram in  " + CurrFileNameInTitle + " has changed\n\nDo you want to save the changes?", "Altalena",
              MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);

            if (result == DialogResult.Yes)
            {
                if (visioControl.Src == String.Empty)
                {
                    DoFileSaveAsCmd();

                }
                else
                {
                    DoFileSaveCmd();
                }
            }

            return result;
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
            if (visioControl.Src == String.Empty)
            {
                DoFileSaveAsCmd();

            }
            else
            {
                DoFileSaveCmd();
            }
            
        }

        private void AltMainMenuStrip_VisibleChanged(object sender, EventArgs e)
        {
            DoVisibleChangedCmd();
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoFileNew();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (visioControl.Src == String.Empty)
            {
                DoFileSaveAsCmd();

            }
            else
            {
                DoFileSaveCmd();
            }
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoFileSaveAsCmd();
        }

        private void printToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoPrintPreviewCmd();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoExitCmd();
        }

        private void printToolStripButton_Click(object sender, EventArgs e)
        {
            DoPrintPreviewCmd();
        }

        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoVisioSelectAllCmd();
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoVisioUndoCmd();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (SaveCheck() == DialogResult.Cancel)
            {
                e.Cancel = true;
                return;
            }
        }

        private void generateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DoGenerateCmd();
        }

        private void convertToScriptButton_Click(object sender, EventArgs e)
        {
            DoGenerateCmd();
        }

#endregion UI_Handlers

#region Commands

#region UICommands
        private void DoVisibleChangedCmd()
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
#endregion UICommands

#region FileCommands

        private void DoFileNew()
        {

            if (SaveCheck() == DialogResult.Cancel)
            {
                return;
            }

            GC.Collect();
            GC.WaitForPendingFinalizers();

            visioControl.Src = "";
            Text = UntitledFileName + TitlePostfix;


            // Get the current state of the visual feedback. If visual
            // feedback is on the end user will see changes in the drawing
            // window as the code operates on the shapes.
            int visualFeedbackStatus = GetApplication().ScreenUpdating;

            // Don't let the user see the visual feedback while the shapes
            // are deleted.
            if (visualFeedbackStatus == visualFeedbackOn)
            {
                GetApplication().ScreenUpdating = visualFeedbackOff;
            }

            // Delete all of the shapes from the Visio window.
            // Note: when invoking Selection.Delete(), the EventSink class will raise
            // an event and call onRemoveProductInformation for each shape deleted
            // from the page. That will update the product data and grid control.
            Window targetWindow;
            targetWindow = (Window)visioControl.Window;
            if (targetWindow.PageAsObj.Shapes.Count > 0)
            {
                targetWindow.SelectAll();
                targetWindow.Selection.Delete();
            }

            // Set the document saved flag since the drawing control is now
            // displaying a blank document.
            visioControl.Document.Saved = true;

            // Restore the screen updating status.
            if (visualFeedbackStatus == visualFeedbackOn)
            {
                GetApplication().ScreenUpdating = visualFeedbackOn;
            }

        }

        private void DoFileOpenCmd(object sender, EventArgs e)
        {

            if (SaveCheck() == DialogResult.Cancel)
            {
                return;
            }

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

                    // user chose stencil
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
                    // user chose visio diagram
                    else if ((fileNameForCompare.IndexOf(VsdSuffix) > 0) ||
                        (fileNameForCompare.IndexOf(VdxSuffix) > 0))
                    {

                        visioControl.Src = "";  

                        // Open the new document.
                        GC.Collect();
                        GC.WaitForPendingFinalizers();

                        visioControl.Src = fileName;
                        visioControl.Document.Saved = true;
                        Text = openFileDialog.SafeFileName + TitlePostfix;


                        setUpVisioDrawing();
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

        private void DoExitCmd()
        {
            

            this.Close();
        }

        private void DoFileSaveCmd()
        {
            GC.Collect();
            GC.WaitForPendingFinalizers();

            string s1 = visioControl.Src;
            bool saved = GetDocument().Saved;

            if (GetDocument().Saved)
            {
                return;
            }

            GC.Collect();
            GC.WaitForPendingFinalizers();

            int res = GetDocument().SaveAs(visioControl.Src);
            GetDocument().Saved = true;

            
        }

        private void DoFileSaveAsCmd()
        {
            
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            DialogResult result = DialogResult.No;

            try
            {
                String targetFilename = visioControl.Src;
                Document targetDocument = GetDocument();

                // Set up the save file dialog and let the user specify the
                // name to save the document to.
                saveFileDialog = new SaveFileDialog();
                saveFileDialog.Title = "Save Script Diagram As";
                saveFileDialog.Filter = "Drawing files (*.vsd)|*.vsd|All files (*.*)|*.*";
                saveFileDialog.FilterIndex = 1;
                saveFileDialog.InitialDirectory = System.Windows.Forms.Application.StartupPath;
                saveFileDialog.FileName = CurrFileNameInTitle;

                result = saveFileDialog.ShowDialog();

                if (result == DialogResult.OK)
                {
                    targetFilename = saveFileDialog.FileName;
                }
                else
                {
                    return;
                }

                targetDocument.SaveAs(targetFilename);

                GC.Collect();
                GC.WaitForPendingFinalizers();

                visioControl.Src = targetFilename;

                Text = System.IO.Path.GetFileName(targetFilename) + TitlePostfix;

                visioControl.Document.Saved = true;

            }
            finally
            {
                // Make sure the dialog is cleaned up.
                if (saveFileDialog != null)
                {
                    saveFileDialog.Dispose();
                }
            }
        }

#endregion FileCommands

#region EditCommands

        private void DoVisioUndoCmd()
        {
            SendCommand(VisUICmds.visCmdEditUndo);
        }

        private void DoVisioSelectAllCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditSelectAll);
        }

        private void DoVisioCopyCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditCopy);
        }

        private void DoVisioCutCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditCut);
        }

        private void DoVisioPasteCmd()
        {
            SendCommand(VisUICmds.visCmdUFEditPaste);
        }        

#endregion EditCommands

#region ScriptCommands
        private void DoValidateCmd()
        {
            VisioDiagramValidator validator =
                new VisioDiagramValidator(GetDocument());

            validator.validate(false);
        }

        private void DoGenerateCmd()
        {
            VisioDiagramValidator validator =
                new VisioDiagramValidator(GetDocument());

            bool valid = validator.validate(true);

            if (!valid)
            {
                return;
            }

            LuaScriptGenerator generator =
                new LuaScriptGenerator(GetDocument());

            generator.generate();

        }

        private void DoShowAboutDlgCmd()
        {
            new AboutForm().Show(this);
        }

        

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
#endregion ScriptCommands

        

      

#endregion Commands

    }
       
}
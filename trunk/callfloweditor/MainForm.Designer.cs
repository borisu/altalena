namespace callfloweditor
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.AltMainMenuStrip = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.BottomToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.TopToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.RightToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.LeftToolStripPanel = new System.Windows.Forms.ToolStripPanel();
            this.ContentPanel = new System.Windows.Forms.ToolStripContentPanel();
            this.toolStripContainer1 = new System.Windows.Forms.ToolStripContainer();
            this.visioControl = new AxMicrosoft.Office.Interop.VisOcx.AxDrawingControl();
            this.mainFormImageList = new System.Windows.Forms.ImageList(this.components);
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.convertToScriptButton = new System.Windows.Forms.ToolStripButton();
            this.validateButton = new System.Windows.Forms.ToolStripButton();
            this.AltMainMenuStrip.SuspendLayout();
            this.toolStripContainer1.ContentPanel.SuspendLayout();
            this.toolStripContainer1.TopToolStripPanel.SuspendLayout();
            this.toolStripContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.visioControl)).BeginInit();
            this.toolStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // AltMainMenuStrip
            // 
            this.AltMainMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.AltMainMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.AltMainMenuStrip.Name = "AltMainMenuStrip";
            this.AltMainMenuStrip.Size = new System.Drawing.Size(785, 24);
            this.AltMainMenuStrip.TabIndex = 0;
            this.AltMainMenuStrip.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // BottomToolStripPanel
            // 
            this.BottomToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.BottomToolStripPanel.Name = "BottomToolStripPanel";
            this.BottomToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.BottomToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.BottomToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // TopToolStripPanel
            // 
            this.TopToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.TopToolStripPanel.Name = "TopToolStripPanel";
            this.TopToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.TopToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.TopToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // RightToolStripPanel
            // 
            this.RightToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.RightToolStripPanel.Name = "RightToolStripPanel";
            this.RightToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.RightToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.RightToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // LeftToolStripPanel
            // 
            this.LeftToolStripPanel.Location = new System.Drawing.Point(0, 0);
            this.LeftToolStripPanel.Name = "LeftToolStripPanel";
            this.LeftToolStripPanel.Orientation = System.Windows.Forms.Orientation.Horizontal;
            this.LeftToolStripPanel.RowMargin = new System.Windows.Forms.Padding(3, 0, 0, 0);
            this.LeftToolStripPanel.Size = new System.Drawing.Size(0, 0);
            // 
            // ContentPanel
            // 
            this.ContentPanel.Size = new System.Drawing.Size(150, 150);
            // 
            // toolStripContainer1
            // 
            // 
            // toolStripContainer1.ContentPanel
            // 
            this.toolStripContainer1.ContentPanel.Controls.Add(this.visioControl);
            this.toolStripContainer1.ContentPanel.Size = new System.Drawing.Size(785, 559);
            this.toolStripContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.toolStripContainer1.Location = new System.Drawing.Point(0, 24);
            this.toolStripContainer1.Name = "toolStripContainer1";
            this.toolStripContainer1.Size = new System.Drawing.Size(785, 584);
            this.toolStripContainer1.TabIndex = 1;
            this.toolStripContainer1.Text = "toolStripContainer1";
            // 
            // toolStripContainer1.TopToolStripPanel
            // 
            this.toolStripContainer1.TopToolStripPanel.Controls.Add(this.toolStrip1);
            // 
            // visioControl
            // 
            this.visioControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.visioControl.Enabled = true;
            this.visioControl.Location = new System.Drawing.Point(0, 0);
            this.visioControl.Name = "visioControl";
            this.visioControl.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("visioControl.OcxState")));
            this.visioControl.Size = new System.Drawing.Size(785, 559);
            this.visioControl.TabIndex = 3;
            // 
            // mainFormImageList
            // 
            this.mainFormImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("mainFormImageList.ImageStream")));
            this.mainFormImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.mainFormImageList.Images.SetKeyName(0, "connection.ico");
            // 
            // toolStrip1
            // 
            this.toolStrip1.Dock = System.Windows.Forms.DockStyle.None;
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.convertToScriptButton,
            this.validateButton});
            this.toolStrip1.Location = new System.Drawing.Point(9, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(89, 25);
            this.toolStrip1.TabIndex = 1;
            // 
            // convertToScriptButton
            // 
            this.convertToScriptButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.convertToScriptButton.Image = ((System.Drawing.Image)(resources.GetObject("convertToScriptButton.Image")));
            this.convertToScriptButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.convertToScriptButton.Name = "convertToScriptButton";
            this.convertToScriptButton.Size = new System.Drawing.Size(23, 22);
            this.convertToScriptButton.Text = "toolStripButton2";
            // 
            // validateButton
            // 
            this.validateButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.validateButton.Image = ((System.Drawing.Image)(resources.GetObject("validateButton.Image")));
            this.validateButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.validateButton.Name = "validateButton";
            this.validateButton.Size = new System.Drawing.Size(23, 22);
            this.validateButton.Text = "toolStripButton2";
            this.validateButton.Click += new System.EventHandler(this.validateButton_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(785, 608);
            this.Controls.Add(this.toolStripContainer1);
            this.Controls.Add(this.AltMainMenuStrip);
            this.MainMenuStrip = this.AltMainMenuStrip;
            this.Name = "MainForm";
            this.Text = "Altalena Call Flow Editor";
            this.AltMainMenuStrip.ResumeLayout(false);
            this.AltMainMenuStrip.PerformLayout();
            this.toolStripContainer1.ContentPanel.ResumeLayout(false);
            this.toolStripContainer1.TopToolStripPanel.ResumeLayout(false);
            this.toolStripContainer1.TopToolStripPanel.PerformLayout();
            this.toolStripContainer1.ResumeLayout(false);
            this.toolStripContainer1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.visioControl)).EndInit();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip AltMainMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripPanel BottomToolStripPanel;
        private System.Windows.Forms.ToolStripPanel TopToolStripPanel;
        private System.Windows.Forms.ToolStripPanel RightToolStripPanel;
        private System.Windows.Forms.ToolStripPanel LeftToolStripPanel;
        private System.Windows.Forms.ToolStripContentPanel ContentPanel;
        private System.Windows.Forms.ToolStripContainer toolStripContainer1;
        private AxMicrosoft.Office.Interop.VisOcx.AxDrawingControl visioControl;
        private System.Windows.Forms.ImageList mainFormImageList;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton convertToScriptButton;
        private System.Windows.Forms.ToolStripButton validateButton;
    }
}


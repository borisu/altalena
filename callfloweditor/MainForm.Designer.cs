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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.AltMainMenuStrip = new System.Windows.Forms.MenuStrip();
            this.visioControl = new AxMicrosoft.Office.Interop.VisOcx.AxDrawingControl();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.AltMainMenuStrip.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.visioControl)).BeginInit();
            this.SuspendLayout();
            // 
            // AltMainMenuStrip
            // 
            this.AltMainMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.AltMainMenuStrip.Location = new System.Drawing.Point(0, 0);
            this.AltMainMenuStrip.Name = "AltMainMenuStrip";
            this.AltMainMenuStrip.Size = new System.Drawing.Size(622, 24);
            this.AltMainMenuStrip.TabIndex = 0;
            this.AltMainMenuStrip.Text = "menuStrip1";
            // 
            // visioControl
            // 
            this.visioControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.visioControl.Enabled = true;
            this.visioControl.Location = new System.Drawing.Point(0, 24);
            this.visioControl.Name = "visioControl";
            this.visioControl.OcxState = ((System.Windows.Forms.AxHost.State)(resources.GetObject("visioControl.OcxState")));
            this.visioControl.Size = new System.Drawing.Size(622, 489);
            this.visioControl.TabIndex = 1;
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(622, 513);
            this.Controls.Add(this.visioControl);
            this.Controls.Add(this.AltMainMenuStrip);
            this.MainMenuStrip = this.AltMainMenuStrip;
            this.Name = "MainForm";
            this.Text = "Altalena Call Flow Editor";
            this.AltMainMenuStrip.ResumeLayout(false);
            this.AltMainMenuStrip.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.visioControl)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip AltMainMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private AxMicrosoft.Office.Interop.VisOcx.AxDrawingControl visioControl;
    }
}


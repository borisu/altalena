using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace callfloweditor
{
    static class EditorMain
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
//             Application.EnableVisualStyles();
//             Application.SetCompatibleTextRenderingDefault(false);
// 
//             MainForm mainForm = new MainForm();
// 
//   
//             
//             
//             Application.Run(mainForm);

            System.Windows.Forms.Application.Run(new MainForm());
        }


        
    }
}
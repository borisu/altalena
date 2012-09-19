using System;
using System.Collections.Generic;
using System.Text;
using iw_cominteropLib;

namespace csharpio
{
    class Program
    {
        
        static void Main(string[] args)
        {

            IComIVRWORX x = new ComIVRWORX();
            x.Init(@"conf.json");
            
            
        }
    }
}

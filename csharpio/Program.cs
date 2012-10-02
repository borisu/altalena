using System;
using System.Collections.Generic;
using System.Text;
using ivrworx;

namespace csharpio
{
    class Program
    {
        
        static void Main(string[] args)
        {
            try
            {
                string fileName = "dotnet.json";
                IvrWORX x = new IvrWORX(fileName);
            }
            catch (System.Exception e)
            {
                System.Console.Out.WriteLine(e);
            	
            }
            
            
            
        }
    }
}

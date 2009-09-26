using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace iw_voicedb
{
    class DataBaseException : Exception
    {
        public DataBaseException(string arg)
            : base(arg)
        {
            
        }

    }
}

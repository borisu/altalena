using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace iw_voicedb
{
    class TypeValidationException : Exception
    {
        public TypeValidationException(string arg):base(arg)
        {
            
        }

    }
}

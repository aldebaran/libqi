using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace QiMessaging
{
    public class Buffer : SafeBuffer
    {
        public Buffer(bool ownsHandle = true) : base(ownsHandle)
        {
        }

        protected override bool ReleaseHandle()
        {
            return true;
        }
    }

}

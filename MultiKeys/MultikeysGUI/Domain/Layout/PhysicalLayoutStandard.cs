using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysGUI.Domain.Layout
{
    public enum PhysicalLayoutStandard
    {
        ANSI = 0,        // US layout, used for some other layouts
        ISO = 1,         // European layout, used for multiple layouts internationally
        ABNT_2 = 2,      // Brazilian portuguese layout
        JIS = 3,         // Japanese layout
        DUBEOLSIK = 4    // Korean layout
    }
}

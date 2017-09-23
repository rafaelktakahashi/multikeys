using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysGUI.Layout
{
    public class PhysicalLayoutFactory
    {
        /// <summary>
        /// Creates a new IPhysicalLayout that conforms to the specified standard.
        /// </summary>
        /// <param name="standard">Physical keyboard standard that determines the shape and existence of certains keys.</param>
        /// <returns></returns>
        public static IPhysicalLayout FromStandard(PhysicalLayoutStandard standard)
        {
            switch (standard)
            {
                case PhysicalLayoutStandard.ANSI:
                    return new ANSIPhysicalLayout();
                case PhysicalLayoutStandard.ISO:
                    return new ISOPhysicalLayout();
                case PhysicalLayoutStandard.ABNT_2:
                    return new ABNTPhysicalLayout();
                case PhysicalLayoutStandard.JIS:
                    return new JISPhysicalLayout();
                case PhysicalLayoutStandard.DUBEOLSIK:
                    throw new NotImplementedException("The Dubeolsik keyboard layout is not implemented yet.");
                default:
                    throw new Exception("Unsupported keyboard layout.");
            }
        }
    }


}

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
                default:
                    throw new NotImplementedException("Other physical layouts will be implemented after the big enters are available.");
            }
        }
    }


}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysEditor.Domain.Layout
{
    public class PhysicalLayoutFactory
    {
        /// <summary>
        /// Creates a new IPhysicalLayout that conforms to the specified standard.
        /// </summary>
        /// <param name="standard">Physical keyboard standard that determines the shape and existence of certains keys.</param>
        /// <param name="useBigReturn">If set to true, the return key will be replaced by a large return key. Has no effect on JIS layout.</param>
        /// <returns></returns>
        public static IPhysicalLayout FromStandard(PhysicalLayoutStandard standard, bool useBigReturn)
        {
            if (useBigReturn)
            {
                // TODO: Put a switch in here
                return new BigReturnANSIPhysicalLayout();
            }

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
                    return new DubeolsikPhysicalLayout();
                default:
                    throw new Exception("Unsupported keyboard layout.");
            }
        }
    }


}

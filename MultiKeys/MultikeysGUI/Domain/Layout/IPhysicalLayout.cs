using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysGUI.Domain.Layout
{

    /// <summary>
    /// A phyical layout represents a collection of physical keys, each associated to a scancode, and
    /// also the size and offset for each key, measured in units. The keys contained in a physical
    /// layout, as well as their sizes, varies depending on physical keyboard standard.
    /// <para/>
    /// Lenghts are measured in units, where the sides of a typical square key are defined as one unit.
    /// <para/>
    /// Every layout has the following dimensions: 6.5 units high, 23 units wide.
    /// </summary>
    public interface IPhysicalLayout
    {
        /// <summary>
        /// Collection of physical keys. Each element carries the location, size and scancode of a physical
        /// key on the keyboard, but not its printed label.
        /// </summary>
        IEnumerable<PhysicalKey> Keys { get; }
    }


}

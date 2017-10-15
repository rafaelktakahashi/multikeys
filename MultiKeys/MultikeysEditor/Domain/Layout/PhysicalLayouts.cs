using MultikeysEditor.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysEditor.Domain.Layout
{
    /// <summary>
    /// This class contains the physical keys that do not change across physical layouts.
    /// This includes function keys, arrow keys, most of the numpad and most of the alphanumeric keys.
    /// <para/>
    /// It is important that whatever class inheriting from this one does not forget to define
    /// each key that is missing. An inheriting class is also free to overwrite any of these definitions.
    /// </summary>
    public abstract class BasePhysicalLayout : IPhysicalLayout
    {
        public BasePhysicalLayout()
        {
            // Initializing each physical key
            //
            // Missing keys             Reason
            // Backspace                Extra 7d key in JIS layout
            // Return key               Different position and shape in non-ANSI layouts
            // 2b (backslash in US)     Different position in non-ANSI layouts
            // Left Shift               Extra 56 key in ISO, ABNT-2 and Dubeolsik layouts
            // Right Shift              Extra 73 key in JIS and ABNT-2 layouts
            // Space                    Multiple extra keys in JIS and Dubeolsik layouts
            // Plus symbol (numpad)     Extra 7e key in ABNT-2 layout
            //
            // The inheriting class must add those keys to this list.
            _layout = new List<PhysicalKey>()
            {
                // Topmost row: Escape key and function keys
                new PhysicalKey("01", 1, 1, 0, 0),    // Esc
                new PhysicalKey("3b", 1, 1, 2, 0),    // F1
                new PhysicalKey("3c", 1, 1, 3, 0),    // F2
                new PhysicalKey("3d", 1, 1, 4, 0),    // F3
                new PhysicalKey("3e", 1, 1, 5, 0),    // F4
                new PhysicalKey("3f", 1, 1, 6.5, 0),  // F5
                new PhysicalKey("40", 1, 1, 7.5, 0),  // F6
                new PhysicalKey("41", 1, 1, 8.5, 0),  // F7
                new PhysicalKey("42", 1, 1, 9.5, 0),  // F8
                new PhysicalKey("43", 1, 1, 11, 0),   // F9
                new PhysicalKey("44", 1, 1, 12, 0),   // F10
                new PhysicalKey("57", 1, 1, 13, 0),   // F11
                new PhysicalKey("58", 1, 1, 14, 0),   // F12
                // First alphanumeric row, except backspace
                new PhysicalKey("29", 1, 1, 0, 1.5),    // Grave
                new PhysicalKey("02", 1, 1, 1, 1.5),    // 1
                new PhysicalKey("03", 1, 1, 2, 1.5),    // 2
                new PhysicalKey("04", 1, 1, 3, 1.5),    // 3
                new PhysicalKey("05", 1, 1, 4, 1.5),    // 4
                new PhysicalKey("06", 1, 1, 5, 1.5),    // 5
                new PhysicalKey("07", 1, 1, 6, 1.5),    // 6
                new PhysicalKey("08", 1, 1, 7, 1.5),    // 7
                new PhysicalKey("09", 1, 1, 8, 1.5),    // 8
                new PhysicalKey("0a", 1, 1, 9, 1.5),    // 9
                new PhysicalKey("0b", 1, 1, 10, 1.5),   // 0
                new PhysicalKey("0c", 1, 1, 11, 1.5),   // Dash
                new PhysicalKey("0d", 1, 1, 12, 1.5),   // Equals
                // Second row, except enter key and the 2b key
                new PhysicalKey("0f", 1.5, 1, 0, 2.5),  // Tab
                new PhysicalKey("10", 1, 1, 1.5, 2.5),  // q
                new PhysicalKey("11", 1, 1, 2.5, 2.5),  // w
                new PhysicalKey("12", 1, 1, 3.5, 2.5),  // e
                new PhysicalKey("13", 1, 1, 4.5, 2.5),  // r
                new PhysicalKey("14", 1, 1, 5.5, 2.5),  // t
                new PhysicalKey("15", 1, 1, 6.5, 2.5),  // y
                new PhysicalKey("16", 1, 1, 7.5, 2.5),  // u
                new PhysicalKey("17", 1, 1, 8.5, 2.5),  // i
                new PhysicalKey("18", 1, 1, 9.5, 2.5),  // o
                new PhysicalKey("19", 1, 1, 10.5, 2.5), // p
                new PhysicalKey("1a", 1, 1, 11.5, 2.5), // [ {
                new PhysicalKey("1b", 1, 1, 12.5, 2.5), // ] }
                // Third row, except enter
                new PhysicalKey("3a", 1.75, 1, 0, 3.5),   // CapsLock
                new PhysicalKey("1e", 1, 1, 1.75, 3.5),   // a
                new PhysicalKey("1f", 1, 1, 2.75, 3.5),   // s
                new PhysicalKey("20", 1, 1, 3.75, 3.5),   // d
                new PhysicalKey("21", 1, 1, 4.75, 3.5),   // f
                new PhysicalKey("22", 1, 1, 5.75, 3.5),   // g
                new PhysicalKey("23", 1, 1, 6.75, 3.5),   // h
                new PhysicalKey("24", 1, 1, 7.75, 3.5),   // j
                new PhysicalKey("25", 1, 1, 8.75, 3.5),   // k
                new PhysicalKey("26", 1, 1, 9.75, 3.5),   // l
                new PhysicalKey("27", 1, 1, 10.75, 3.5),  // Semicolon
                new PhysicalKey("28", 1, 1, 11.75, 3.5),  // Apostrophe
                // Fourth row, except both shifts
                new PhysicalKey("2c", 1, 1, 2.25, 4.5),   // z
                new PhysicalKey("2d", 1, 1, 3.25, 4.5),   // x
                new PhysicalKey("2e", 1, 1, 4.25, 4.5),   // c
                new PhysicalKey("2f", 1, 1, 5.25, 4.5),   // v
                new PhysicalKey("30", 1, 1, 6.25, 4.5),   // b
                new PhysicalKey("31", 1, 1, 7.25, 4.5),   // n
                new PhysicalKey("32", 1, 1, 8.25, 4.5),   // m
                new PhysicalKey("33", 1, 1, 9.25, 4.5),   // Comma
                new PhysicalKey("34", 1, 1, 10.25, 4.5),  // Dot
                new PhysicalKey("35", 1, 1, 11.25, 4.5),  // Slash
                // Fifth row, except space bar
                new PhysicalKey("1d", 1.25, 1, 0, 5.5),         // LCtrl
                new PhysicalKey("e0:5b", 1.25, 1, 1.25, 5.5),   // LWin
                new PhysicalKey("38", 1.25, 1, 2.5, 5.5),       // LAlt
                new PhysicalKey("e0:38", 1.25, 1, 10, 5.5),     // RAlt
                new PhysicalKey("e0:5c", 1.25, 1, 11.25, 5.5),  // RWin
                new PhysicalKey("e0:5d", 1.25, 1, 12.5, 5.5),   // Menu
                new PhysicalKey("e0:1d", 1.25, 1, 13.75, 5.5),  // RCtrl
                // Arrow keys and such
                new PhysicalKey("e0:37", 1, 1, 15.5, 0),        // PrintScreen / SysReq
                new PhysicalKey("46", 1, 1, 16.5, 0),           // ScrollLock
                new PhysicalKey("e1:1d", 1, 1, 17.5, 0),        // Pause / Break
                new PhysicalKey("e0:52", 1, 1, 15.5, 1.5),      // Insert
                new PhysicalKey("e0:47", 1, 1, 16.5, 1.5),      // Home
                new PhysicalKey("e0:49", 1, 1, 17.5, 1.5),      // PageUp
                new PhysicalKey("e0:53", 1, 1, 15.5, 2.5),      // Delete
                new PhysicalKey("e0:4f", 1, 1, 16.5, 2.5),      // End
                new PhysicalKey("e0:51", 1, 1, 17.5, 2.5),      // PageDown
                new PhysicalKey("e0:48", 1, 1, 16.5, 4.5),      // Up Arrow
                new PhysicalKey("e0:4b", 1, 1, 15.5, 5.5),      // Left Arrow
                new PhysicalKey("e0:50", 1, 1, 16.5, 5.5),      // Down Arrow
                new PhysicalKey("e0:4d", 1, 1, 17.5, 5.5),      // Right Arrow
                // Numpad keys, except plus symbol
                new PhysicalKey("45", 1, 1, 19, 1.5),           // NumLock
                new PhysicalKey("e0:35", 1, 1, 20, 1.5),        // Slash
                new PhysicalKey("37", 1, 1, 21, 1.5),           // Asterisk
                new PhysicalKey("4a", 1, 1, 22, 1.5),           // Minus
                new PhysicalKey("47", 1, 1, 19, 2.5),           // 7 / Home
                new PhysicalKey("48", 1, 1, 20, 2.5),           // 8 / Up Arrow
                new PhysicalKey("49", 1, 1, 21, 2.5),           // 9, PageUp
                new PhysicalKey("4b", 1, 1, 19, 3.5),           // 4 / Left Arrow
                new PhysicalKey("4c", 1, 1, 20, 3.5),           // 5
                new PhysicalKey("4d", 1, 1, 21, 3.5),           // 6 / Right Arrow
                new PhysicalKey("4f", 1, 1, 19, 4.5),           // 1 / End
                new PhysicalKey("50", 1, 1, 20, 4.5),           // 2 / Arrow Down
                new PhysicalKey("51", 1, 1, 21, 4.5),           // 3 / Page Down
                new PhysicalKey("e0:1c", 1, 2, 22, 4.5),        // Enter
                new PhysicalKey("52", 2, 1, 19, 5.5),           // 0 / Insert
                new PhysicalKey("53", 1, 1, 21, 5.5)            // . / Delete
            };
        }

        /// <summary>
        /// For internal use only; any class that inherits from this
        /// must use this field in order to modify the layout.
        /// A class inheriting from this is permitted to edit the field,
        /// but taking care that no scancodes are repeated.
        /// </summary>
        protected IList<PhysicalKey> _layout;

        /// <summary>
        /// Exposed property.
        /// </summary>
        public IEnumerable<PhysicalKey> Keys { get { return _layout; } }
    }




    public class ANSIPhysicalLayout : BasePhysicalLayout
    {
        public ANSIPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 2, 1, 13, 1.5));       // 2-wide backspace
            _layout.Add(new PhysicalKey("2b", 1.5, 1, 13.5, 2.5));   // 1.5-wide backslash
            _layout.Add(new PhysicalKey("1c", 2.25, 1, 12.75, 3.5)); // 2.25-wide horizontal return
            _layout.Add(new PhysicalKey("2a", 2.25, 1, 0, 4.5));     // 2.25-wide left shift
            _layout.Add(new PhysicalKey("36", 2.75, 1, 12.25, 4.5)); // 2.75-wide right shift
            _layout.Add(new PhysicalKey("39", 6.25, 1, 3.75, 5.5));  // full-width (6.25) space bar
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }

    public class ISOPhysicalLayout : BasePhysicalLayout
    {
        public ISOPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 2, 1, 13, 1.5));       // 2-wide backspace
            _layout.Add(new PhysicalKey("1c", 1.5, 2, 13.5, 2.5,     // L-shaped Return key
                PhysicalKeyShape.LShapedReturn));
            _layout.Add(new PhysicalKey("2b", 1, 1, 12.75, 3.5));    // 1x1 key to the left of return
            _layout.Add(new PhysicalKey("2a", 1.25, 1, 0, 4.5));     // 1.25-wide left shift
            _layout.Add(new PhysicalKey("56", 1, 1, 1.25, 4.5));        // 1x1 key to the right of left shift
            _layout.Add(new PhysicalKey("36", 2.75, 1, 12.25, 4.5)); // 2.75-wide right shift
            _layout.Add(new PhysicalKey("39", 6.25, 1, 3.75, 5.5));  // full-width (6.25) space bar
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }

    public class ABNTPhysicalLayout : BasePhysicalLayout
    {
        public ABNTPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 2, 1, 13, 1.5));       // 2-wide backspace
            _layout.Add(new PhysicalKey("1c", 1.5, 2, 13.5, 2.5,     // L-shaped Return key
                PhysicalKeyShape.LShapedReturn));
            _layout.Add(new PhysicalKey("2b", 1, 1, 12.75, 3.5));    // 1x1 key to the left of return
            _layout.Add(new PhysicalKey("2a", 1.25, 1, 0, 4.5));     // 1.25-wide left shift
            _layout.Add(new PhysicalKey("56", 1, 1, 1.25, 4.5));     // 1x1 key to the right of left shift (backslash)
            _layout.Add(new PhysicalKey("73", 1, 1, 12.25, 4.5));    // 1x1 key to the left of right shift (slash)
            _layout.Add(new PhysicalKey("36", 1.75, 1, 13.25, 4.5)); // 1.75-wide right shift
            _layout.Add(new PhysicalKey("39", 6.25, 1, 3.75, 5.5));  // full-width (6.25) space bar
            _layout.Add(new PhysicalKey("4e", 1, 1, 22, 2.5));       // 1-tall plus symbol
            _layout.Add(new PhysicalKey("7e", 1, 1, 22, 3.5));       // Numpad thousands separator (dot)
        }
    }

    public class JISPhysicalLayout : BasePhysicalLayout
    {
        public JISPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("7d", 1, 1, 13, 1.5));       // 1x1 key to the left of backspace (Yen)
            _layout.Add(new PhysicalKey("0e", 1, 1, 14, 1.5));       // 1-wide backspace
            _layout.Add(new PhysicalKey("1c", 1.5, 2, 13.5, 2.5,     // L-shaped Return key
                PhysicalKeyShape.LShapedReturn));
            _layout.Add(new PhysicalKey("2b", 1, 1, 12.75, 3.5));    // 1x1 key to the left of return
            _layout.Add(new PhysicalKey("2a", 2.25, 1, 0, 4.5));     // 2.25-wide left shift
            _layout.Add(new PhysicalKey("73", 1, 1, 12.25, 4.5));    // 1x1 key to the left of right shift
            _layout.Add(new PhysicalKey("36", 1.75, 1, 13.25, 4.5)); // 1.75-wide right shift
            _layout.Add(new PhysicalKey("7b", 1, 1, 3.75, 5.5));     // Mukenhan key to the left of space bar
            _layout.Add(new PhysicalKey("39", 3.25, 1, 4.75, 5.5));  // reduced-width (3.5) space bar
            _layout.Add(new PhysicalKey("79", 1, 1, 8, 5.5));        // Henkan key to the right of space bar
            _layout.Add(new PhysicalKey("70", 1, 1, 9, 5.5));        // Kana key to the right of henkan
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }

    public class DubeolsikPhysicalLayout : BasePhysicalLayout
    {
        public DubeolsikPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 2, 1, 13, 1.5));       // 2-wide backspace
            _layout.Add(new PhysicalKey("1c", 1.5, 2, 13.5, 2.5,     // L-shaped Return key
                PhysicalKeyShape.LShapedReturn));
            _layout.Add(new PhysicalKey("2b", 1, 1, 12.75, 3.5));    // 1x1 key to the left of return
            _layout.Add(new PhysicalKey("2a", 1.25, 1, 0, 4.5));     // 1.25-wide left shift
            _layout.Add(new PhysicalKey("56", 1, 1, 1.25, 4.5));     // 1x1 key to the right of left shift
            _layout.Add(new PhysicalKey("36", 2.75, 1, 12.25, 4.5)); // 2.75-wide right shift
            _layout.Add(new PhysicalKey("f1", 1, 1, 3.75, 5.5));     // Hanja key to the left of space bar
            _layout.Add(new PhysicalKey("39", 4.25, 1, 4.75, 5.5));  // reduced-width (4.25) space bar
            _layout.Add(new PhysicalKey("f2", 1, 1, 9, 5.5));        // Han/yeong key to the right of space bar
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }

    public class BigReturnANSIPhysicalLayout : BasePhysicalLayout
    {
        public BigReturnANSIPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 1, 1, 14, 1.5));       // 1-wide backspace
            _layout.Add(new PhysicalKey("2b", 1, 1, 13, 1.5));     // 1-wide backslash
            _layout.Add(new PhysicalKey("1c", 2.25, 2, 12.75, 2.5,   // Big return
                PhysicalKeyShape.BigReturn));
            _layout.Add(new PhysicalKey("2a", 2.25, 1, 0, 4.5));     // 2.25-wide left shift
            _layout.Add(new PhysicalKey("36", 2.75, 1, 12.25, 4.5)); // 2.75-wide right shift
            _layout.Add(new PhysicalKey("39", 6.25, 1, 3.75, 5.5));  // full-width (6.25) space bar
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }

    public class BigReturnISOPhysicalLayout : BasePhysicalLayout
    {
        public BigReturnISOPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 1, 1, 14, 1.5));       // 1-wide backspace
            _layout.Add(new PhysicalKey("2b", 1, 1, 13, 1.5));     // 1-wide 2b key
            _layout.Add(new PhysicalKey("1c", 2.25, 2, 12.75, 2.5,   // Big return
                PhysicalKeyShape.BigReturn));
            _layout.Add(new PhysicalKey("2a", 1.25, 1, 0, 4.5));     // 1.25-wide left shift
            _layout.Add(new PhysicalKey("56", 1, 1, 1.25, 4.5));        // 1x1 key to the right of left shift
            _layout.Add(new PhysicalKey("36", 2.75, 1, 12.25, 4.5)); // 2.75-wide right shift
            _layout.Add(new PhysicalKey("39", 6.25, 1, 3.75, 5.5));  // full-width (6.25) space bar
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }

    public class BigReturnABNTPhysicalLayout : BasePhysicalLayout
    {
        public BigReturnABNTPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 1, 1, 14, 1.5));       // 1-wide backspace
            _layout.Add(new PhysicalKey("2b", 1, 1, 13, 1.5));     // 1-wide 2b key
            _layout.Add(new PhysicalKey("1c", 2.25, 2, 12.75, 2.5,   // Big return
                PhysicalKeyShape.BigReturn));
            _layout.Add(new PhysicalKey("2a", 1.25, 1, 0, 4.5));     // 1.25-wide left shift
            _layout.Add(new PhysicalKey("56", 1, 1, 1.25, 4.5));     // 1x1 key to the right of left shift (backslash)
            _layout.Add(new PhysicalKey("73", 1, 1, 12.25, 4.5));    // 1x1 key to the left of right shift (slash)
            _layout.Add(new PhysicalKey("36", 1.75, 1, 13.25, 4.5)); // 1.75-wide right shift
            _layout.Add(new PhysicalKey("39", 6.25, 1, 3.75, 5.5));  // full-width (6.25) space bar
            _layout.Add(new PhysicalKey("4e", 1, 1, 22, 2.5));       // 1-tall plus symbol
            _layout.Add(new PhysicalKey("7e", 1, 1, 22, 3.5));       // Numpad thousands separator (dot)
        }
    }

    // No support for big return JIS layout

    public class BigReturnDubeolsikPhysicalLayout : BasePhysicalLayout
    {
        public BigReturnDubeolsikPhysicalLayout() : base()
        {
            _layout.Add(new PhysicalKey("0e", 1, 1, 14, 1.5));       // 1-wide backspace
            _layout.Add(new PhysicalKey("2b", 1, 1, 13, 1.5));     // 1-wide 2b key
            _layout.Add(new PhysicalKey("1c", 2.25, 2, 12.75, 2.5,   // Big return
                PhysicalKeyShape.BigReturn));
            _layout.Add(new PhysicalKey("2a", 1.25, 1, 0, 4.5));     // 1.25-wide left shift
            _layout.Add(new PhysicalKey("56", 1, 1, 1.25, 4.5));     // 1x1 key to the right of left shift
            _layout.Add(new PhysicalKey("36", 2.75, 1, 12.25, 4.5)); // 2.75-wide right shift
            _layout.Add(new PhysicalKey("f1", 1, 1, 3.75, 5.5));     // Hanja key to the left of space bar
            _layout.Add(new PhysicalKey("39", 4.25, 1, 4.75, 5.5));  // reduced-width (4.25) space bar
            _layout.Add(new PhysicalKey("f2", 1, 1, 9, 5.5));        // Han/yeong key to the right of space bar
            _layout.Add(new PhysicalKey("4e", 1, 2, 22, 2.5));       // 2-tall plus symbol
        }
    }


}

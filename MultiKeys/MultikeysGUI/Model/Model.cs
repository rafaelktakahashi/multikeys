using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysGUI.Model
{
    /// <summary>
    /// Uniquely represents each physical key on a keyboard.
    /// </summary>
    public class Scancode
    {
        /// <summary>
        /// Constructs a new scancode from a string.
        /// </summary>
        /// <param name="sc">A 16-bit integer, represented in hexadecimal
        /// in the form [0x]AA[:]BB.
        /// AA may be either E0 or E1. BB is interpreted as the make code.</param>
        public Scancode (string sc)
            : this(
                  UInt16.Parse(
                      new string(sc.Where(c => !char.IsPunctuation(c) && !char.IsWhiteSpace(c)).ToArray()),
                      System.Globalization.NumberStyles.HexNumber)
                  )
        { }

        /// <summary>
        /// Constructs a new scancode from a value.
        /// </summary>
        /// <param name="sc">A 16-bit integer. The least significative 8 bits
        /// are interpreted as the make code, while the most significative
        /// 8 bits are interpreted as either an e0 or an e1 flag.</param>
        public Scancode(UInt16 sc)
            : this(
                    (sc >> 8) == 0xe0,
                    (sc >> 8) == 0xe1,
                    (byte)(sc & 0xff)
                  )
        { }

        public Scancode(bool e0, bool e1, byte makeCode)
        {
            this.E0 = e0;
            this.E1 = e1;
            this.MakeCode = makeCode;
        }

        public Scancode() { }

        /// <summary>
        /// Flag that indicates a scancode prefixed with the byte 0xE0.
        /// </summary>
        public bool E0 { get; set; }
        /// <summary>
        /// Flag that indicates a scancode prefixed with the byte 0xE1.
        /// </summary>
        public bool E1 { get; set; }
        /// <summary>
        /// Byte for the make code, in scancode set 2.
        /// </summary>
        public byte MakeCode { get; set; }

        public override string ToString()
        {
            if (E0) return "E0" + MakeCode.ToString("X2");
            else if (E1) return "E1" + MakeCode.ToString("X2");
            else return MakeCode.ToString("X2");
        }
    }

    public class VirtualKeystroke
    {
        public VirtualKeystroke() { }

        public byte VirtualKeyCode { get; set; }
        public bool KeyDown { get; set; }
    }

    /// <summary>
    /// Root element; contains a list of keyboards.
    /// </summary>
    public class MultikeysRoot
    {
        public IList<Keyboard> Keyboards { get; set; }
    }

    /// <summary>
    /// Each remapped keyboard registered by the user.
    /// </summary>
    public class Keyboard
    {
        /// <summary>
        /// Name of device, as reported by the RawInput API.
        /// </summary>
        public string UniqueName { get; set; }
        /// <summary>
        /// Readable name given to the keyboard; this string is chosen by the user,
        /// and has no effect on functionality.
        /// </summary>
        public string Alias { get; set; }
        /// <summary>
        /// Modifiers that are registered on this keyboard.
        /// </summary>
        public IList<Modifier> Modifiers { get; set; }
        /// <summary>
        /// Levels registered on this keyboard; each is activated by a particular combination of modifiers.
        /// </summary>
        public IList<Level> Levels { get; set; }
    }

    /// <summary>
    /// Modifier key, uniquely identified by a scancode.
    /// Alternatively, more than one physical key might correspond
    /// to the same logical modifier (e.g. Shift and Ctrl keys on a normal keyboard).
    /// </summary>
    public class Modifier
    {
        /// <summary>
        /// Name of this modifier; this is mandatory, and is used to uniquely identify modifiers.
        /// </summary>
        public string Name { get; set; }
        /// <summary>
        /// Physical keys that are part of this modifier.<para/>
        /// More than one physical key may be associated with the same modifier;<para/>
        /// in that case, the modifier counts as pressed down if one or more phyical keys
        /// associated with it are pressed down.
        /// </summary>
        public IList<Scancode> Scancodes { get; set; }
    }

    /// <summary>
    /// Group of commands that correspond to a certain combination of modifiers pressed down.
    /// An empty list of modifiers is a valid combination.
    /// A combination of modifiers may not correspond to more than one level.
    /// </summary>
    public class Level
    {
        /// <summary>
        /// Readable name given to the level; this string is chosen by the user, and has no effect on functionality.
        /// </summary>
        public string Alias { get; set; }
        /// <summary>
        /// Combination of modifiers that must be pressed down on the keyboard in order to activate this level.<para/>
        /// Each modifier registered on the keyboard that is present in this list must be pressed down.<para/>
        /// Each modifier registered on the keyboard that is not present in this list must be released.
        /// Modifiers are identified by name.
        /// </summary>
        public IList<string> ModifierCombination { get; set; }
        /// <summary>
        /// List of remaps in this level.
        /// A remap is associated with a physical key, identified by scancode.
        /// Each scancode may be associated with at most one command. <para/>
        /// Each stored command also contains the scancode.
        /// </summary>
        public IDictionary<Scancode, Command> Commands { get; set; }
    }

    /// <summary>
    /// Base class for a command.
    /// Commands represent actions taken in response to a user keypress.
    /// </summary>
    public abstract class Command
    {
        public Scancode Scancode { get; set; }
    }

    /// <summary>
    /// Command that sends one or more Unicode characters
    /// to the window that would receive the original keystroke.
    /// </summary>
    public class UnicodeCommand : Command
    {
        /// <summary>
        /// Whether or not this command should activate multiple times if the user keeps the key pressed down.
        /// </summary>
        public bool TriggerOnRepeat { get; set; }
        /// <summary>
        /// List of Unicode characters, identified by codepoint, that should be simulated.
        /// </summary>
        public IList<UInt32> Codepoints { get; set; }
    }

    /// <summary>
    /// Command that sends one or more simulated keytrokes
    /// to the window that would receive the original keystroke.
    /// </summary>
    public class MacroCommand : Command
    {
        /// <summary>
        /// Whether or not this command should activate multiple times if the user keeps the key pressed down.
        /// </summary>
        public bool TriggerOnRepeat { get; set; }
        /// <summary>
        /// List of virtual keys that should be simulated.
        /// </summary>
        public IList<VirtualKeystroke> VKeyCodes { get; set; }
    }

    /// <summary>
    /// Command that executes a certain file in disk, with optional arguments.
    /// </summary>
    public class ExecutableCommand : Command
    {
        /// <summary>
        /// Command that should be executed on keypress.<para/>
        /// This may be a fully qualified path to a file that can be executed.
        /// </summary>
        public string Command { get; set; }
        /// <summary>
        /// Arguments to pass to the command. May be null.
        /// </summary>
        public string Arguments { get; set; }
    }

    /// <summary>
    /// Special type of Unicode command that replicates the behavior
    /// of a dead key. Only works for replacing Unicode commands.
    /// </summary>
    public class DeadKeyCommand : Command
    {
        /// <summary>
        /// Unicode character(s) associated with this dead key.
        /// </summary>
        public IList<UInt32> Codepoints { get; set; }
        /// <summary>
        /// List of replacements contained in this dead key.<para/>
        /// The command pressed after this key is compared to the first element in this dictionary.<para/>
        /// If it matches any key, its corresponding value will substitute the original command.
        /// </summary>
        public IDictionary<IList<UInt32>, IList<UInt32>> Replacements { get; set; }
    }
}

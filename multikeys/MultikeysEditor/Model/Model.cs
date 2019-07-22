using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Input;

namespace MultikeysEditor.Model
{
    /// <summary>
    /// Uniquely represents each physical key on a keyboard.
    /// </summary>
    public class Scancode
    {
        /// <summary>
        /// Constructs a new scancode from a string.
        /// Note: The Pause/Break key is identified by e1 1d instead of its full sequence (e1 1d 45).
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

        public static Scancode FromString(string sc)
        {
            return new Scancode(
                  UInt16.Parse(
                      sc.Replace(":", ""),
                      System.Globalization.NumberStyles.HexNumber)
                  );
        }

        /// <summary>
        /// Constructs a new scancode from a value.
        /// </summary>
        /// <param name="sc">A 16-bit integer. The least significative 8 bits
        /// are interpreted as the make code, while the most significative
        /// 8 bits are interpreted as either an e0 or an e1 flag.</param>
        public Scancode(UInt16 sc)
            : this(
                    (sc & 0xff00) == 0xe000,
                    (sc & 0xff00) == 0xe100,
                    (byte)(sc & 0xff)
                  )
        { }

        public Scancode(bool e0, bool e1, byte makeCode)
        {
            this.E0 = e0;
            this.E1 = e1;
            this.Code = makeCode;
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
        public byte Code { get; set; }

        public override string ToString()
        {
            if (E0) return "E0" + ':' + Code.ToString("X2");
            else if (E1) return "E1" + ':' + Code.ToString("X2");
            else return Code.ToString("X2");
        }

        public static bool operator ==(Scancode x, Scancode y)
        {
            if (ReferenceEquals(x, null) ^ ReferenceEquals(y, null)) return false;
            if (ReferenceEquals(x, null) && ReferenceEquals(y, null)) return true;
            return (x.E0 == y.E0) && (x.E1 == y.E1) && (x.Code == y.Code);
        }
        public static bool operator !=(Scancode x, Scancode y)
        {
            if (ReferenceEquals(x, null) ^ ReferenceEquals(y, null)) return true;
            if (ReferenceEquals(x, null) && ReferenceEquals(y, null)) return false;
            return (x.E0 != y.E0) || (x.E1 != y.E1) || (x.Code != y.Code);
        }
        public override bool Equals(object obj)
        {
            if (!(obj is Scancode)) return false;
            else return this == (obj as Scancode);
        }
        public override int GetHashCode()
        {
            return this.Code % 32;  // don't worry too much about it.
        }
    }

    public class VirtualKeystroke
    {
        public VirtualKeystroke() { }

        public byte VirtualKeyCode { get; set; }
        public bool KeyDown { get; set; }

        public string VirtualKeyName
        {
            get
            {
                return KeyInterop.KeyFromVirtualKey(VirtualKeyCode).ToString();
            }
        }

        public string KeyDownName
        {
            get
            {
                return KeyDown ? Properties.Strings.MacroKeyDown : Properties.Strings.MacroKeyUp;
            }
        }

        public override string ToString()
        {
            return $"{Properties.Strings.VirtualKeyCode}: {VirtualKeyCode.ToString("X")};" +
                $"({(KeyDown ? Properties.Strings.KeyPress : Properties.Strings.KeyRelease)})";
        }
    }

    /// <summary>
    /// Root element; contains a list of keyboards.
    /// </summary>
    public class MultikeysLayout
    {
        public MultikeysLayout()
        { Keyboards = new List<Keyboard>(); }

        public IList<Keyboard> Keyboards { get; set; }
    }

    /// <summary>
    /// Each remapped keyboard registered by the user.
    /// </summary>
    public class Keyboard
    {
        public Keyboard()
        {
            // The keyboard constructor must create at least one layer.
            UniqueName = string.Empty;
            Alias = null;
            Modifiers = new List<Modifier>();
            Layers = new List<Layer>
            {
                new Layer()
            };
        }

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
        /// Layers registered on this keyboard; each is activated by a particular combination of modifiers.
        /// </summary>
        public IList<Layer> Layers { get; set; }
        /// <summary>
        /// Name of the physical layout that the user chose to display; not used in Multikeys Core.
        /// </summary>
        public Domain.Layout.PhysicalLayoutStandard PhysicalLayout { get; set; }
        /// <summary>
        /// Name of the logical layout that the user chose to display; not used in Multikeys Core.
        /// </summary>
        public string LogicalLayout { get; set; }
    }

    /// <summary>
    /// Modifier key, uniquely identified by a scancode.
    /// Alternatively, more than one physical key might correspond
    /// to the same logical modifier (e.g. Shift and Ctrl keys on a normal keyboard).
    /// </summary>
    public class Modifier : IComparable<Modifier>
    {
        public Modifier()
        {
            Name = null;
            Scancodes = new List<Scancode>();
        }

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

        public int CompareTo(Modifier other)
        {
            return this.Name.CompareTo(other.Name);
        }
    }

    /// <summary>
    /// Group of commands that correspond to a certain combination of modifiers pressed down.
    /// An empty list of modifiers is a valid combination.
    /// A combination of modifiers may not correspond to more than one layer.
    /// </summary>
    public class Layer
    {
        public Layer()
        {
            Alias = null;
            ModifierCombination = new List<string>();
            Commands = new Dictionary<Scancode, IKeystrokeCommand>();
        }

        /// <summary>
        /// Readable name given to the layer; this string is chosen by the user, and has no effect on functionality.
        /// </summary>
        public string Alias { get; set; }
        /// <summary>
        /// Combination of modifiers that must be pressed down on the keyboard in order to activate this layer.<para/>
        /// Each modifier registered on the keyboard that is present in this list must be pressed down.<para/>
        /// Each modifier registered on the keyboard that is not present in this list must be released.
        /// Modifiers are identified by name.
        /// </summary>
        public IList<string> ModifierCombination { get; set; }
        /// <summary>
        /// List of remaps in this layer.
        /// A remap is associated with a physical key, identified by scancode.
        /// Each scancode may be associated with at most one command. <para/>
        /// Each stored command also contains the scancode.
        /// </summary>
        public IDictionary<Scancode, IKeystrokeCommand> Commands { get; set; }
    }

    /// <summary>
    /// Base class for a command.
    /// Commands represent actions taken in response to a user keypress.
    /// </summary>
    public interface IKeystrokeCommand { }

    /// <summary>
    /// Command that sends one or more Unicode characters
    /// to the window that would receive the original keystroke.
    /// </summary>
    public class UnicodeCommand : IKeystrokeCommand
    {
        // Default constructor
        public UnicodeCommand() : this(false, "") { }

        // Constructor from text
        public UnicodeCommand(bool triggerOnRepeat, string text)
        {
            TriggerOnRepeat = triggerOnRepeat;
            Codepoints = _stringToCodepoints(text);
        }

        protected IList<UInt32> _stringToCodepoints(string input)
        {
            var result = new List<UInt32>();
            /*
             * In C# (and in Windows in general), each Unicode char is a UTF-16 code unit,
             * which is 16 bits in length. Each Unicode character is represented by either
             * one or two code units (pairs of code units are called surrogate pairs).
             * char.ConvertToUtf32(string s, int index) reads either one or two chars at
             * the specified index and produces a 32-bit integer, the codepoint. If the
             * codepoint is larger than 0xffff (half of a 32-bit integer's capacity), we
             * know it was read from two UTF-16 code units (two chars), so we skip the next
             * char.
             */
            for (int i = 0; i < input.Length; i++)
            {
                int codepoint = char.ConvertToUtf32(input, i);
                if (codepoint > 0xffff)
                { i++; }
                result.Add((UInt32)codepoint);
            }
            return result;
        }

        /// <summary>
        /// Whether or not this command should activate multiple times if the user keeps the key pressed down.
        /// </summary>
        public bool TriggerOnRepeat { get; set; }
        /// <summary>
        /// List of Unicode characters, identified by codepoint, that should be simulated.
        /// </summary>
        public IList<UInt32> Codepoints { get; set; }

        public string ContentAsText
        {
            get
            {
                var builder = new StringBuilder();
                foreach (UInt32 codepoint in Codepoints)
                    builder.Append(
                        char.ConvertFromUtf32((int)codepoint)
                        );
                return builder.ToString();
            }
        }
    }

    /// <summary>
    /// Command that sends one or more simulated keytrokes
    /// to the window that would receive the original keystroke.
    /// </summary>
    public class MacroCommand : IKeystrokeCommand
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
    public class ExecutableCommand : IKeystrokeCommand
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
    public class DeadKeyCommand : UnicodeCommand
    {
        public DeadKeyCommand() { }

        public DeadKeyCommand(string text, IList<ReplacementPair> replacements) : base(false, text)
        {
            // Each item of replacements contains a 'from' string and a 'to' string, and both need
            // to be converted to lists of 32-bit integers, because that's what the core understands.
            Replacements = new Dictionary<IList<UInt32>, IList<UInt32>>();
            foreach (var replacementPair in replacements)
            {
                Replacements.Add(_stringToCodepoints(replacementPair.From), _stringToCodepoints(replacementPair.To));
            }
        }
        

        /// <summary>
        /// Map of replacements contained in this dead key, only works for substituting Unicode commands.<para/>
        /// The command pressed after this key is compared to each key in the dictionary.<para/>
        /// If it matches any key, its corresponding value will substitute the original command.<para/>
        /// </summary>
        public IDictionary<IList<UInt32>, IList<UInt32>> Replacements { get; set; }
        /// <summary>
        /// Keep in mind that these model classes generally represent text as lists of codepoints, because
        /// that's how Multikeys core represents it. This returns the replacements as objects containing strings.
        /// </summary>
        public IList<ReplacementPair> ReplacementsAsList
        {
            get
            {
                var result = new List<ReplacementPair>();
                foreach (var key in Replacements.Keys)
                {
                    IList<UInt32> from = key;
                    IList<UInt32> to = Replacements[key];
                    result.Add(new ReplacementPair(from, to));
                }
                return result;
            }
        }



        /// <summary>
        /// For easier display of text.
        /// </summary>
        public class ReplacementPair
        {
            public ReplacementPair(IList<UInt32> from, IList<UInt32> to)
            {
                var sb = new StringBuilder();
                foreach (UInt32 codepoint in from)
                    sb.Append(char.ConvertFromUtf32((int)codepoint));
                From = sb.ToString();

                sb.Clear();
                foreach (UInt32 codepoint in to)
                    sb.Append(char.ConvertFromUtf32((int)codepoint));
                To = sb.ToString();
            }
            public ReplacementPair(string from, string to)
            {
                From = from;
                To = to;
            }
            public string From { get; set; }
            public string To { get; set; }
        }
    }
    
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using MultikeysEditor.Model;

namespace MultikeysGUITests
{
    public static class ModelExtensions
    {
        public static string Print(this MultikeysLayout @this)
        {
            return $"{@this.Keyboards.Select(it => it.Print()).JoinIntoString()}";
        }
        public static string Print(this Keyboard @this)
        {
            return
                $"\n{@this.Alias}: {@this.UniqueName}\n"
                + $"Modifiers:\n{@this.Modifiers.Select(it => it.Print()).JoinIntoString()}\n"
                + $"Layers:\n{@this.Layers.Select(it => it.Print()).JoinIntoString()}\n";
        }
        public static string Print(this Modifier @this)
        {
            return $"    {@this.Name}: {@this.Scancodes.Select(it => it.Code.ToString()).JoinIntoString()}\n";
        }
        public static string Print(this Layer @this)
        {
            return $"    Layer {@this.Alias}\n" +
                $"    Activates with: {@this.ModifierCombination.JoinIntoString()}\n" +
                $"    Contains: {@this.Commands.Select(it => it.Value.Print()).JoinIntoString()}\n";
        }
        public static string Print(this IKeystrokeCommand @this)
        {
            if (@this is UnicodeCommand) return (@this as UnicodeCommand).Print();
            else if (@this is MacroCommand) return (@this as MacroCommand).Print();
            else if (@this is ExecutableCommand) return (@this as ExecutableCommand).Print();
            else if (@this is DeadKeyCommand) return (@this as DeadKeyCommand).Print();
            else return "        Non-printable command\n";
        }
        public static string Print(this UnicodeCommand @this)
        {
            return $"{@this.Codepoints.Select(it => $"{it.ToString()} ({it.AsCharacter()})").JoinIntoStringManyLines("            ")}\n";
        }
        public static string Print(this MacroCommand @this)
        {
            return $"{@this.VKeyCodes.Select(it => it.VirtualKeyCode.ToString()).JoinIntoStringManyLines("            ")}\n";
        }
        public static string Print(this ExecutableCommand @this)
        {
            return $"            {@this.Command} {@this.Arguments}\n";
        }
        public static string Print(this DeadKeyCommand @this)
        {
            return $"            Printed form currently unavailable.\n";
        }

        private static string JoinIntoString(this IEnumerable<string> strings)
        {
            return string.Join(" ", strings.ToArray());
        }
        private static string JoinIntoStringManyLines(this IEnumerable<string> strings, string prefix)
        {
            return string.Join("\n", strings.Select(it => prefix + it).ToArray());
        }

        private static string AsCharacter(this UInt32 codepoint)
        {
            return char.ConvertFromUtf32((int)codepoint);
        }
    }
}

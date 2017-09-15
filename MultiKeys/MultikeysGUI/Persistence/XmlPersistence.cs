using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.IO;
using System.Xml;
using System.Xml.Linq;
using System.Xml.Schema;
using MultikeysGUI.Model;

namespace MultikeysGUI.Persistence
{
    /// <summary>
    /// Reads and writes xml configuration files
    /// according to the model.
    /// </summary>
    public class XmlPersistence
    {

        private static readonly XmlSchemaSet schemaSet;

        static XmlPersistence()
        {
            try
            {
                // Initialize the schema set:
                schemaSet = new XmlSchemaSet();

                // Create the reader from a stream containing the xsd (which is an embedded resource)
                XmlReader reader = XmlReader.Create(
                    typeof(XmlPersistence).Assembly.GetManifestResourceStream("MultikeysGUI.Resources.Multikeys.xsd")
                    );

                // Add the xsd contained in the reader to the schema set.
                schemaSet.Add("", reader);

            }
            catch (Exception)
            {
                schemaSet = null;
            }
        }



        /// <summary>
        /// Opens and reads an xml file at <paramref name="filename"/>,
        /// and loads its contents into a new model.
        /// </summary>
        /// <param name="filename">Fully qualified path to the configuration file to be read.</param>
        /// <returns>An object containing the contents of the configuration file.</returns>
        public static MultikeysRoot Load(string filename)
        {
            if (filename == null)
            { throw new ArgumentNullException("Filename provided was null."); }

            if (string.IsNullOrWhiteSpace(filename))
            { throw new ArgumentException("Filename provided was empty."); }

            // This method should fail if this class' schema set failed to initialize
            if (schemaSet == null)
            { throw new Exception("The XML schema file failed to load."); }


            // Prepare the document
            XDocument document = XDocument.Load(filename);

            var reader = XmlReader.Create(filename);

            // Validate the document
            document.Validate(schemaSet, (obj, e) =>
            {
                throw new XmlSchemaValidationException(e.Message);
            });

            // Read the document
            return ReadConfiguration(document);

        }

        // Methods for reading elements


        private static VirtualKeystroke ReadVKey(XElement el)
        {
            return new VirtualKeystroke
            {
                KeyDown = el.Attribute("Keypress").Value == "Down",
                VirtualKeyCode = byte.Parse(el.Value, System.Globalization.NumberStyles.HexNumber)
            };
        }

        private static UInt32 ReadCodepoint(XElement el)
        {
            return UInt32.Parse(el.Value, System.Globalization.NumberStyles.HexNumber);
        }

        private static UnicodeCommand ReadUnicodeCommand(XElement el)
        {
            return new UnicodeCommand
            {
                Scancode = new Scancode(el.Attribute("Scancode").Value),
                TriggerOnRepeat = el.Attribute("TriggerOnRepeat").Value == "True",
                Codepoints = el.Elements("codepoint").Select(it => ReadCodepoint(it)).ToList()
            };
        }

        private static MacroCommand ReadMacroCommand(XElement el)
        {
            return new MacroCommand
            {
                Scancode = new Scancode(el.Attribute("Scancode").Value),
                TriggerOnRepeat = el.Attribute("TriggerOnRepeat").Value == "True",
                VKeyCodes = el.Elements("vkey").Select(it => ReadVKey(it)).ToList()
            };
        }

        private static ExecutableCommand ReadExecutableCommand(XElement el)
        {
            return new ExecutableCommand
            {
                Scancode = new Scancode(el.Attribute("Scancode").Value),
                Command = el.Element("path").Value,
                Arguments = el.Element("parameter")?.Value
            };
        }

        private static Tuple<IList<UInt32>, IList<UInt32>> ReadReplacement(XElement el)
        {
            var first = el.Element("from").Elements("codepoint").Select(it => ReadCodepoint(it)).ToList();
            var second = el.Element("to").Elements("codepoint").Select(it => ReadCodepoint(it)).ToList();
            return new Tuple<IList<UInt32>, IList<UInt32>>(first, second);
        }

        private static DeadKeyCommand ReadDeadKeyCommand(XElement el)
        {
            return new DeadKeyCommand
            {
                Scancode = new Scancode(el.Attribute("Scancode").Value),
                Codepoints = el.Element("independent").Elements("codepoint").Select(it => ReadCodepoint(it)).ToList(),
                Replacements = el.Elements("replacement").Select(it => ReadReplacement(it)).ToDictionary(it => it.Item1, it => it.Item2)
            };
        }

        private static Tuple<Scancode, Command> ReadCommand(XElement el)
        {
            Command second = null;
            switch (el.Name.LocalName)
            {
                case "unicode":
                    second = ReadUnicodeCommand(el);
                    break;
                case "macro":
                    second = ReadMacroCommand(el);
                    break;
                case "execute":
                    second = ReadExecutableCommand(el);
                    break;
                case "deadkey":
                    second = ReadDeadKeyCommand(el);
                    break;
                default:
                    throw new Exception("Unrecognized command element.");
            }

            var first = new Scancode(el.Attribute("Scancode").Value);

            return new Tuple<Scancode, Command>(first, second);

        }

        private static Level ReadLevel(XElement el)
        {
            return new Level
            {
                ModifierCombination = el.Elements("modifier").Select(it => it.Value).ToList(),
                Commands = (
                            from element in el.Elements()
                            where element.Name.LocalName != "modifier"
                            select ReadCommand(element)
                            ).ToDictionary(x => x.Item1, x => x.Item2)
            };
        }

        private static IList<Modifier> ReadModifiers(XElement el)
        {
            return
            (
                from modifier in el.Elements("modifier")
                group modifier by modifier.Attribute("Name").Value into modifierGroup
                select new Modifier
                {
                    Name = modifierGroup.First().Attribute("Name").Value,
                    Scancodes = (
                                                from modComponent in modifierGroup
                                                select new Scancode(modComponent.Value)
                                                ).ToList()
                }
            ).ToList();
        }

        private static Keyboard ReadKeyboard(XElement el)
        {
            return new Keyboard
            {
                UniqueName = el.Attribute("Name").Value,
                Alias = el.Attribute("Alias")?.Value,   // may be null
                Modifiers = ReadModifiers(el.Element("modifiers")),
                Levels = el.Elements("level").Select(it => ReadLevel(it)).ToList()
            };
        }

        private static MultikeysRoot ReadConfiguration(XDocument doc)
        {
            return new MultikeysRoot
            {
                Keyboards = doc.Descendants("keyboard").Select(it => ReadKeyboard(it)).ToList()
            };
        }



        /// <summary>
        /// Creates and writes to a file at <paramref name="filename"/>,
        /// containing the data in <paramref name="content"/>.
        /// </summary>
        /// <param name="content">Object containing the data to be written.</param>
        /// <param name="filename">Fully qualified path where the configuration file will be saved.</param>
        public static void Save(MultikeysRoot content, string filename)
        {
            WriteConfiguration(content).Save(filename);
        }

        // Methods for writing the model into xml
        private static XElement WriteVKey(VirtualKeystroke entity)
        {
            return new XElement(
                "vkey",
                new XAttribute("Keypress", entity.KeyDown ? "Down" : "Up"),
                entity.VirtualKeyCode.ToString("X2")
                );
        }

        private static XElement WriteCodepoint(UInt32 entity)
        {
            return new XElement(
                "codepoint",
                entity.ToString("X")
                );
        }

        private static XElement WriteUnicodeCommand(UnicodeCommand entity)
        {
            var element = new XElement("unicode");
            element.Add(new XAttribute("Scancode", entity.Scancode.ToString()));
            element.Add(new XAttribute("TriggerOnRepeat", entity.TriggerOnRepeat ? "True" : "False"));
            foreach (var codepoint in entity.Codepoints)
            {
                element.Add(WriteCodepoint(codepoint));
            }

            return element;
        }

        private static XElement WriteMacroCommand(MacroCommand entity)
        {
            var element = new XElement("macro");
            element.Add(new XAttribute("Scancode", entity.Scancode.ToString()));
            element.Add(new XAttribute("TriggerOnRepeat", entity.TriggerOnRepeat ? "True" : "False"));
            foreach (var vkey in entity.VKeyCodes)
            {
                element.Add(WriteVKey(vkey));
            }

            return element;
        }

        private static XElement WriteExecutableCommand(ExecutableCommand entity)
        {
            var element = new XElement("execute");
            element.Add(new XAttribute("Scancode", entity.Scancode.ToString()));

            element.Add(new XElement("path", entity.Command));
            if (entity.Arguments != null)
                element.Add(new XElement("parameter", entity.Arguments));

            return element;
        }

        private static XElement WriteReplacement(Tuple<IList<UInt32>, IList<UInt32>> entity)
        {
            var element = new XElement("replacement");

            var element_1 = new XElement("from");
            foreach (UInt32 codepoint in entity.Item1)
            {
                element_1.Add(WriteCodepoint(codepoint));
            }

            var element_2 = new XElement("to");
            foreach (UInt32 codepoint in entity.Item2)
            {
                element_2.Add(WriteCodepoint(codepoint));
            }

            element.Add(element_1);
            element.Add(element_2);
            return element;
        }

        private static XElement WriteDeadKeyCommand(DeadKeyCommand entity)
        {
            var element = new XElement("deadkey");
            element.Add(new XAttribute("Scancode", entity.Scancode.ToString()));
            var element_1 = new XElement("independent");
            foreach (UInt32 codepoint in entity.Codepoints)
            {
                element_1.Add(WriteCodepoint(codepoint));
            }
            element.Add(element_1);
            foreach (var replacement in entity.Replacements)
            {
                element.Add(WriteReplacement(new Tuple<IList<UInt32>, IList<UInt32>>(replacement.Key, replacement.Value)));
            }
            return element;
        }

        private static XElement WriteCommand(Command entity)
        {
            // must check more specific types first, if using "is" keyword
            if (entity is DeadKeyCommand) return WriteDeadKeyCommand(entity as DeadKeyCommand);
            if (entity is MacroCommand) return WriteMacroCommand(entity as MacroCommand);
            if (entity is UnicodeCommand) return WriteUnicodeCommand(entity as UnicodeCommand);
            if (entity is ExecutableCommand) return WriteExecutableCommand(entity as ExecutableCommand);
            return new XElement("command", "ERROR WRITING COMMAND");
        }

        private static XElement WriteLevel(Level entity)
        {
            var element = new XElement("level");
            element.Add(new XAttribute("Alias", entity.Alias));
            foreach (string modifierName in entity.ModifierCombination)
            {
                element.Add(new XElement("modifier", modifierName));
            }
            foreach (var command in entity.Commands.Select(c => c.Value))
            {
                element.Add(WriteCommand(command));
            }
            return element;
        }

        private static XElement WriteModifiers(IList<Modifier> entity)
        {
            var element = new XElement("modifiers");
            foreach (Modifier modifier in entity)
            {
                foreach (Scancode scancode in modifier.Scancodes)
                {
                    var element_1 = new XElement("modifier", scancode.ToString());
                    element_1.Add(new XAttribute("Name", modifier.Name));
                }
            }
            return element;
        }

        private static XElement WriteKeyboard(Keyboard entity)
        {
            var element = new XElement("keyboard");
            element.Add(new XAttribute("Name", entity.UniqueName));
            if (entity.Alias != null)
                element.Add(new XAttribute("Alias", entity.Alias));
            element.Add(WriteModifiers(entity.Modifiers));
            foreach (var level in entity.Levels)
            {
                element.Add(WriteLevel(level));
            }
            return element;
        }

        private static XElement WriteConfiguration(MultikeysRoot entity)
        {
            var element = new XElement("Multikeys");
            foreach (var keyboard in entity.Keyboards)
            {
                element.Add(WriteKeyboard(keyboard));
            }
            return element;
        }
        



    }
}

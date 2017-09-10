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
            document.Validate(schemaSet, (obj, e) => {
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
        
        private static  UInt32 ReadCodepoint(XElement el)
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
                    return null;
            }

            var first = new Scancode(el.Attribute("Scancode").Value);

            return new Tuple<Scancode, Command>(first, second);

        }
        
        private static Level ReadLevel(XElement el)
        {
            return new Level
            {
                ModifierCombination = el.Elements("modifier").Select(it => it.Value).ToList(),
                Commands = el.Elements().Select(it => ReadCommand(it)).Where(it => it != null).ToDictionary(x => x.Item1, x=>x.Item2)
            };
        }
        
        private static IList<Modifier> ReadModifiers(XElement el)
        {
            var modifiers = new List<Modifier>();
            foreach (var node in el.Elements("modifier"))
            {
                string modifierName = node.Attribute("Name").Value;
                // if another modifier with the same name was already read, add this scancode to it
                if (modifiers.Any(m => m.Name == modifierName))
                {
                    modifiers.Find(m => m.Name == modifierName).Scancodes.Add(
                        new Scancode(node.Value)
                        );
                    continue;
                }
                // otherwise, make a new modifier
                var mod = new Modifier
                {
                    Name = modifierName,
                    Scancodes = new List<Scancode> { new Scancode(node.Value) }
                };
                modifiers.Add(mod);
            }
            return modifiers;
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
            // Create 
        }


    }
}

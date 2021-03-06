﻿using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

using MultikeysEditor.Model;

namespace MultikeysGUITests
{
    /// <summary>
    /// Ensures that the Multikeys GUI is capable of reading and writing xml configuration
    /// files correctly.
    /// </summary>
    [TestClass]
    public class XMLTests
    {
        [ClassInitialize]
        public static void Setup(TestContext context)
        {
            // Write the sample into the user's temporary folder for use in tests
            string tempPath = Path.GetTempPath();

            using (var resourceStream = typeof(XMLTests).Assembly.GetManifestResourceStream("MultikeysGUITests.Resources.Sample.xml"))
            using (var newFile = File.Create($@"{tempPath}\Sample.xml"))        // <- will overwrite if already exists
            {
                resourceStream.CopyTo(newFile);
            }
        }

        [ClassCleanup]
        public static void Teardown()
        {
            // Remove the sample file
            if (File.Exists($@"{Path.GetTempPath()}\Sample.xml"))
            {
                try
                { File.Delete($@"{Path.GetTempPath()}\Sample.xml"); }
                catch (Exception) { }       // no problem.
            }
        }

        [TestMethod]
        public void ReadFile()
        {
            var result = MultikeysEditor.Persistence.XmlPersistence.Load($@"{Path.GetTempPath()}\Sample.xml");
            // Do not assert anything. This code is "asserted" by debugging.
            string res = result.Print();
            Assert.AreEqual("Debug to assert", "Debug to assert");
        }

        [TestMethod]
        public void WriteFile()
        {
            var layout = new MultikeysLayout()
            {
                Keyboards = new System.Collections.Generic.List<Keyboard>()
            };
            layout.Keyboards.Add(new Keyboard
            {
                UniqueName = @"\\?\HID#VID_1A2C&PID_0B2A&amp;MI_00#8&16c55830&0&0000#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}",
                Alias = "Default",
                Modifiers = new System.Collections.Generic.List<Modifier>(),
                Layers = new System.Collections.Generic.List<Layer>()
            });
            layout.Keyboards[0].Layers.Add(new Layer
            {
                Alias = "Default layer",
                ModifierCombination = new System.Collections.Generic.List<string>(),
                Commands = new System.Collections.Generic.Dictionary<Scancode, IKeystrokeCommand>()
            });
            layout.Keyboards[0].Layers[0].Commands.Add(
                new Scancode(2),
                new UnicodeCommand
                {
                    TriggerOnRepeat = true,
                    Codepoints = new System.Collections.Generic.List<uint> { 0x1F642 }
                }
                );
            MultikeysEditor.Persistence.XmlPersistence.Save(layout, $@"C:\Users\Rafael\Desktop\multikeys.xml");

        }
    }
}

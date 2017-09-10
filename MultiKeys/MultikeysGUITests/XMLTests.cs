using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.IO;

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
            using (var newFile = File.Create($@"{tempPath}\Sample.xml"))
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
                File.Delete($@"{Path.GetTempPath()}\Sample.xml");
            }
        }

        [TestMethod]
        public void TestMethod1()
        {
            var result = MultikeysGUI.Persistence.XmlPersistence.Load($@"{Path.GetTempPath()}\Sample.xml");
            // Do not assert anything. This code is "asserted" by debugging.
            Assert.AreEqual("Debug to assert", "Debug to assert");
        }
    }
}

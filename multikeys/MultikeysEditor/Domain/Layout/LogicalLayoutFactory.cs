﻿using MultikeysEditor.Model;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

namespace MultikeysEditor.Domain.Layout
{
    public static class LogicalLayoutFactory
    {

        /// <summary>
        /// Retrieves a dictionary from scancodes to their labels, according to a logical keyboard layout.
        /// </summary>
        /// <param name="logicalLayoutName">
        /// Name of the keyboard; a file by this name must exist in the Resources\Layouts folder;
        /// do not pass the file extension in this parameter.
        /// </param>
        public static IDictionary<Scancode, string> GetLogicalLayout(string logicalLayoutName)
        {
            var layout = new Dictionary<Scancode, string>();

            string pathToLayout =
                System.IO.Path
                .GetDirectoryName(
                    System.Reflection.Assembly
                    .GetEntryAssembly()
                    .Location
                    )
                    + $@"\Resources\Layouts\{logicalLayoutName}.layout";

            if (!File.Exists(pathToLayout))
            {
                // Fallback
                return new Dictionary<Scancode, string>();
            }

            // hold each line
            var lines = new List<string>();

            // read each line of file
            using (var reader = new StreamReader(pathToLayout))
            {
                // hold one line
                var line = "";
                while (!reader.EndOfStream)
                {
                    line = reader.ReadLine();
                    if (!string.IsNullOrWhiteSpace(line))
                    { lines.Add(line); }
                }
            }

            foreach (string line in lines)
            {
                // The old algorithm did not allow for spaces in the text after the scancode.

                //// Apparently, compiling a regex results in execution that is a little bit faster,
                //// but is orders of magniture slower to construct. Benchmarking is welcome, but I do
                //// not believe it's necessary to worry too much about this.
                //var both = Regex.Replace(line, "(?=.*)\\s(?=.*)", "!").Split('!');
                //string scancode = both[0];
                //// name might be empty, in which case an empty string should be used
                //string name = (both.Length <= 1 ? "" : both[1]);
                //layout.Add(Scancode.FromString(scancode), name);

                // Line is in format:
                // e0:37 PrtSc
                // first whitespace separates scancode from text.

                // first, split by whitespace (string.Split uses whitespace when passed null)
                // also, limit the split by 2, so that only the first encountered 
                string[] words = line.Split(null as char[], 2);
                // remove any line that didn't have whitespace at all.
                if (words.Length <= 1) continue;
                // scancode is the first word, text is the second word
                layout.Add(Scancode.FromString(words[0]), words[1]);
            }

            return layout;
        }
    }
}

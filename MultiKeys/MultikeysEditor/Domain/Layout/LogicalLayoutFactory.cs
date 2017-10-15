using MultikeysEditor.Model;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace MultikeysEditor.Domain.Layout
{
    public static class LogicalLayoutFactory
    {
        public static IDictionary<Scancode, string> GetLogicalLayout(LogicalLayout logicalLayout)
        {
            var layout = new Dictionary<Scancode, string>();

            // Load a sample layout for now
            // In the future, this should be configurable
            string pathToLayout =
                System.IO.Path
                .GetDirectoryName(
                    System.Reflection.Assembly
                    .GetEntryAssembly()
                    .Location
                    )
                    + @"\Resources\Layouts\US.layout";

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
                // Apparently, compiling a regex results in execution that is a little bit faster,
                // but is orders of magniture slower to construct. Benchmarking is welcome, but I do
                // not believe it's necessary to worry too much about this.
                var both = Regex.Replace(line, "(?=.*)\\s(?=.*)", "!").Split('!');
                string scancode = both[0];
                // name might be empty, in which case an empty string should be used
                string name = (both.Length <= 1 ? "" : both[1]);
                layout.Add(Scancode.FromString(scancode), name);
            }

            return layout;
        }
    }
}

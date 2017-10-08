using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysGUI.Domain.RawInput
{
    public static class RawKeyboard
    {
        /// <summary>
        /// Waits for a keystroke from the user, then returns the name of that keyboard.
        /// </summary>
        /// <returns>The unique name of the device as a string.</returns>
        public static Task<string> DetectKeyboardName()
        {

            string pathToExecutable =
                System.IO.Path
                .GetDirectoryName(
                    System.Reflection.Assembly
                    .GetEntryAssembly()
                    .Location
                    )
                    + @"\Resources\DetectKeyboardName.exe";

            try
            {
                Process p = new Process
                {
                    StartInfo =
                    {
                        FileName = pathToExecutable,
                        RedirectStandardOutput = true,
                        CreateNoWindow = true,      // the executable should not create a window anyway
                        UseShellExecute = false,
                    },
                };
                
                p.Start();
                p.WaitForExit();

                StreamReader reader = p.StandardOutput;
                string kbName =
                    reader.ReadToEnd()
                    .Replace(Environment.NewLine, string.Empty);

                return Task.FromResult(kbName);

            } catch (Exception e)
            {
                // Do some exection handling
                throw e;
            }

        }
    }
}

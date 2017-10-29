using System;
using System.Diagnostics;
using System.IO;

namespace MultikeysEditor.Domain.BackgroundRunner
{
    public enum MultikeysCoreState
    {
        Stopped = 0,
        Running = 1,
        Paused = 2,
    }

    class MultikeysCoreRunner
    {
        public MultikeysCoreRunner()
        {
            string pathToCore =
                System.IO.Path
                .GetDirectoryName(
                    System.Reflection.Assembly
                    .GetEntryAssembly()
                    .Location
                    )
                    + @"\Resources\MultikeysCore.exe";

            // Look for a process already running
            // TODO

            mkCoreProcess =
                new Process
                {
                    StartInfo =
                    {
                        FileName = pathToCore,
                        CreateNoWindow = true,
                    }
                };

            State = MultikeysCoreState.Stopped;
        }
        ~MultikeysCoreRunner()
        {
            try { mkCoreProcess.Kill(); } catch { }
        }

        /// <summary>
        /// The current state of the background process.
        /// </summary>
        public MultikeysCoreState State { get; private set; }

        private string _pathToLayout;
        /// <summary>
        /// Path to a Multikeys file layout.
        /// This property must be set in order to start the core application.
        /// </summary>
        public string PathToLayout
        {
            get { return _pathToLayout; }
            set
            {
                _pathToLayout = value;
                mkCoreProcess.StartInfo.Arguments = value;
            }
        }

        private Process mkCoreProcess;

        /// <summary>
        /// Starts running the background process, using the informed layout.
        /// </summary>
        public void Start(string pathToLayout)
        {
            if (!File.Exists(pathToLayout))
            {
                throw new ArgumentException("The specified layout file was not found.");
            }
            mkCoreProcess.StartInfo.Arguments = '"' + pathToLayout + '"';
            mkCoreProcess.Start();
            State = MultikeysCoreState.Running;
        }

        /// <summary>
        /// Stops running the background process.
        /// </summary>
        public void Stop()
        {
            // The core process is a Win32 window application, and it accepts a close message:
            // mkCoreProcess.CloseMainWindow();

            if (!mkCoreProcess.HasExited)
            {
                mkCoreProcess.Kill();
            }
        }
    }
}

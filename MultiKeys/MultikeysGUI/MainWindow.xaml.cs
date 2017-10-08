using Microsoft.Win32;
using MultikeysGUI.Domain;
using MultikeysGUI.Domain.BackgroundRunner;
using MultikeysGUI.Domain.Layout;
using MultikeysGUI.Model;
using MultikeysGUI.View.Controls;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;


namespace MultikeysGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            EnableDisableMenuButtons();

            // Setup the core runner and the icons
            multikeysCoreRunner = new MultikeysCoreRunner();
        }

        /// <summary>
        /// The currently opened file; null if no file is open.
        /// If this variable contains an empty string, then the file being
        /// edited has not been saved yet.
        /// </summary>
        private string workingFileName = null;

        /// <summary>
        /// The control that represents the layout being currently edited.
        /// </summary>
        private LayoutControl layoutControl = null;



        #region Menu

        private void FileNew_Click(object sender, EventArgs e)
        {
            // If there is already a file being edited
            if (workingFileName != null)
            {
                // Ask the user if they want to discard the open file
                var result = MessageBox.Show(Properties.Strings.WarningWillOverwriteWorkingLayout,
                    Properties.Strings.Warning,
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Warning);
                if (result != MessageBoxResult.Yes)
                { return; }
            }

            // Make a brand new layout instance, with a keyboard in it
            var newLayout = new MultikeysLayout();
            newLayout.Keyboards.Add(new Model.Keyboard());
            // empty string means layout that hasn't been saved yet
            workingFileName = "";
            // Place a new layout control in the dock panel
            layoutControl = new LayoutControl();
            layoutControl.LoadLayout(newLayout);
            DockPanelLayout.Children.Clear();
            DockPanelLayout.Children.Add(layoutControl);

            EnableDisableMenuButtons();
        }

        private void FileSave_Click(object sender, EventArgs e)
        {
            // Can't save a layout if there's none being edited
            if (workingFileName == null)
            {
                MessageBox.Show(Properties.Strings.WarningNoLayout, Properties.Strings.Warning, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            else if (workingFileName == "")
            {
                // Empty string means the file hasn't been saved yet
                // Do the same as "save as"
                FileSaveAs_Click(sender, e);
                return;
            }
            else
            {
                new DomainFacade().SaveLayout(layoutControl.GetLayout(), workingFileName);
            }

            EnableDisableMenuButtons();
        }

        private void FileSaveAs_Click(object sender, EventArgs e)
        {
            // Can't save a layout if there's none being edited
            if (workingFileName == null)
            {
                MessageBox.Show(Properties.Strings.WarningNoLayout, Properties.Strings.Warning, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            // Prompt for a filename to save
            var saveFileDialog = new SaveFileDialog();
            if (saveFileDialog.ShowDialog() == true)
            {
                // retrieve layout and save it
                new DomainFacade().SaveLayout(layoutControl.GetLayout(), saveFileDialog.FileName);
                // then, the new filenamed is used as the current file being edited
                workingFileName = saveFileDialog.FileName;
            }

            EnableDisableMenuButtons();
        }

        private void FileOpen_Click(object sender, EventArgs e)
        {
            // If there is a layout being edited
            if (workingFileName != null)
            {
                // Warn the user that their changes will be lost, prompt for cancellation
                var result = MessageBox.Show(Properties.Strings.WarningWillOverwriteWorkingLayout,
                    Properties.Strings.Warning,
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Warning);
                if (result != MessageBoxResult.Yes)
                { return; }
            }

            // Ask for a file to open
            var openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                // TODO: Add exception handling when opening files
                var layout = new DomainFacade().LoadLayout(openFileDialog.FileName);
                // remember what file is being worked on
                workingFileName = openFileDialog.FileName;
                // and replace the layout control
                layoutControl = new LayoutControl();
                layoutControl.LoadLayout(layout);
                DockPanelLayout.Children.Clear();
                DockPanelLayout.Children.Add(layoutControl);
            }

            EnableDisableMenuButtons();
        }

        private void FileClose_Click(object sender, EventArgs e)
        {
            // If there's no file to close:
            if (workingFileName == null)
            {
                MessageBox.Show(Properties.Strings.WarningNoLayout, Properties.Strings.Warning, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            // Confirm closing
            var result = MessageBox.Show(Properties.Strings.WarningWillOverwriteWorkingLayout,
                    Properties.Strings.Warning,
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Warning);
            if (result == MessageBoxResult.No)
            { return; }

            // Just remove the current layout
            layoutControl = null;
            DockPanelLayout.Children.Clear();

            EnableDisableMenuButtons();
        }


        /// <summary>
        /// Although the menu items event handlers have validation in them, it's useful to disable actions that the user
        /// should not do, such as closing the current layout when there is no open layout.
        /// </summary>
        private void EnableDisableMenuButtons()
        {
            // when there's no open layout
            if (workingFileName == null)
            {
                MenuFileNew.IsEnabled = true;
                MenuFileSave.IsEnabled = false;
                MenuFileSaveAs.IsEnabled = false;
                MenuFileOpen.IsEnabled = true;
                MenuFileClose.IsEnabled = false;
            }
            // when there's an open layout, all options are enabled
            else
            {
                MenuFileNew.IsEnabled = true;
                MenuFileSave.IsEnabled = true;
                MenuFileSaveAs.IsEnabled = true;
                MenuFileOpen.IsEnabled = true;
                MenuFileClose.IsEnabled = true;
            }
        }

        #endregion



        #region BackgroundRunner

        private MultikeysCoreRunner multikeysCoreRunner;

        private void BackgroundRunnerIconStart_Loaded(object sender, RoutedEventArgs e)
        {
            var uriSource = new Uri(@"Resources/StateRunning.png", UriKind.Relative);

            (sender as Image).Source = new BitmapImage(uriSource);
        }

        private void BackgroundRunnerIconStop_Loaded(object sender, RoutedEventArgs e)
        {
            var uriSource = new Uri(@"Resources/StateStopped.png", UriKind.Relative);

            (sender as Image).Source = new BitmapImage(uriSource);
        }


        private void BackgroundRunnerIconStart_MouseUp(object sender, MouseButtonEventArgs e)
        {
            // If the file name does not yet exist (that is, not yet saved), then warn the user
            // and save it.
            if (!File.Exists(workingFileName))
            {
                var result = MessageBox.Show(
                    Properties.Strings.WarningSaveLayoutToStart,
                    Properties.Strings.Warning,
                    MessageBoxButton.YesNo,
                    MessageBoxImage.Warning);
                if (result == MessageBoxResult.Yes)
                {
                    FileSaveAs_Click(null, null);
                }
                else return;
            }
            multikeysCoreRunner.Start(workingFileName);
            BackgroundRunnerIconStart.Visibility = Visibility.Hidden;
            BackgroundRunnerIconStop.Visibility = Visibility.Visible;
        }

        private void BackgroundRunnerIconStop_MouseUp(object sender, MouseButtonEventArgs e)
        {
            multikeysCoreRunner.Stop();
            BackgroundRunnerIconStart.Visibility = Visibility.Visible;
            BackgroundRunnerIconStop.Visibility = Visibility.Hidden;
        }


        #endregion


    }
}

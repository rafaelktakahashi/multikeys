using Microsoft.Win32;
using MultikeysGUI.Domain;
using MultikeysGUI.Domain.Layout;
using MultikeysGUI.Model;
using MultikeysGUI.View.Controls;
using System;
using System.Collections.Generic;
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
        }

        /// <summary>
        /// The currently opened file; null if no file is open.
        /// </summary>
        private string workingFileName = null;

        /// <summary>
        /// The control that represents the layout being currently edited.
        /// </summary>
        private LayoutControl layoutControl = null;



        #region Menu

        private void FileNew_Click(object sender, EventArgs e)
        {
            ;
        }

        private void FileSave_Click(object sender, EventArgs e)
        {
            ;
        }

        private void FileSaveAs_Click(object sender, EventArgs e)
        {
            if (workingFileName == null)
            {
                MessageBox.Show(Properties.Strings.WarningNoLayout, Properties.Strings.Warning, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            var saveFileDialog = new SaveFileDialog();
            if (saveFileDialog.ShowDialog() == true)
            {
                // retrieve layout and save it
                new DomainFacade().SaveLayout(layoutControl.GetLayout(), saveFileDialog.FileName);
                workingFileName = saveFileDialog.FileName;
            }
        }

        private void FileOpen_Click(object sender, EventArgs e)
        {


            var openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                var layout = new DomainFacade().LoadLayout(openFileDialog.FileName);
                workingFileName = openFileDialog.FileName;
                layoutControl = new LayoutControl();
                layoutControl.LoadLayout(layout);
                dockPanel.Children.Clear();
                dockPanel.Children.Add(layoutControl);
            }
        }

        private void FileClose_Click(object sender, EventArgs e)
        {
            // Confirm closing

        }

        #endregion



    }
}

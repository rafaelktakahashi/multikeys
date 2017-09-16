using MultikeysGUI.Model;
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
using System.Windows.Shapes;

namespace MultikeysGUI.Pages.Dialogues
{
    /// <summary>
    /// Interaction logic for UnicodeCommandDialog.xaml
    /// </summary>
    public partial class UnicodeCommandDialog : Window
    {
        public UnicodeCommandDialog()
        {
            InitializeComponent();
            Command = null;
        }

        /// <summary>
        /// This property will contain the command once this window is closed,
        /// or null if operation was cancelled.
        /// </summary>
        public UnicodeCommand Command { get; private set; }

        private void ChangeSize(object sender, RoutedEventArgs e)
        {
            if (checkBoxLarge.IsChecked ?? false)   // should never be indeterminate
            {
                // use large version of dialog
            }
            else
            {
                // use small version of dialog
            }
        }

        private void Submit(object sender, RoutedEventArgs e)
        {
            Command = new UnicodeCommand(true, txtInput.Text);
            DialogResult = true;
        }
        


    }
}

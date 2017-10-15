using MultikeysEditor.Domain;
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

namespace MultikeysEditor.View.Dialogues
{
    /// <summary>
    /// Interaction logic for KeyboardNameDialog.xaml
    /// </summary>
    public partial class KeyboardNameDialog : Window
    {
        public KeyboardNameDialog(string currentName)
        {
            InitializeComponent();

            LabelKeyboardName.Content = currentName == "" ? Properties.Strings.KeyboardNameDialogAnyKeyboard : currentName;
            KeyboardName = currentName;
        }

        /// <summary>
        /// This property will contain the unique name of a device in case this window's ShowDialog()
        /// returns true. If the user did not choose a unique name, an empty string is returned,
        /// representing any keyboard. If this window was not properly closed, then this property is null.
        /// </summary>
        public string KeyboardName { get; private set; }

        private void ButtonListenForInput_Click(object sender, RoutedEventArgs e)
        {
            Task.Run(ButtonListenForInput_Click_Async);
        }

        private async Task ButtonListenForInput_Click_Async()
        {
            Dispatcher.Invoke(() =>
            {
                LabelPressAnyKey.Visibility = Visibility.Visible;
            });
            
            string kbName = await new DomainFacade().DetectKeyboardUniqueName();

            Dispatcher.Invoke(() =>
            {
                LabelKeyboardName.Content = kbName;
                KeyboardName = kbName;
                LabelPressAnyKey.Visibility = Visibility.Hidden;
            });
            
        }

        private void ButtonUseAnyKeyboard_Click(object sender, RoutedEventArgs e)
        {
            LabelKeyboardName.Content = Properties.Strings.KeyboardNameDialogAnyKeyboard;
            KeyboardName = string.Empty;
        }

        private void ButtonFinish_Click(object sender, RoutedEventArgs e)
        {
            if (KeyboardName == null)
            { KeyboardName = string.Empty; }

            DialogResult = true;    // causes window to close successfully
        }
    }
}

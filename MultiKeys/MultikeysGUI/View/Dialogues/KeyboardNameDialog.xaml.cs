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

namespace MultikeysGUI.View.Dialogues
{
    /// <summary>
    /// Interaction logic for KeyboardNameDialog.xaml
    /// </summary>
    public partial class KeyboardNameDialog : Window
    {
        public KeyboardNameDialog(string currentName)
        {
            InitializeComponent();

            LabelKeyboardName.Content = currentName;
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
            // TODO: Place real implementation here
            LabelKeyboardName.Content = @"\\?\HID#VID_1A2C&PID_0B2A&MI_00#8&16c55830&0&0000#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}";
            KeyboardName = @"\\?\HID#VID_1A2C&PID_0B2A&MI_00#8&16c55830&0&0000#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}";
        }

        private void ButtonUseAnyKeyboard_Click(object sender, RoutedEventArgs e)
        {
            LabelKeyboardName.Content = string.Empty;
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

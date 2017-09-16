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

using MultikeysGUI.Model;
using System.ComponentModel;
using MultikeysGUI.Pages.Dialogues;

namespace MultikeysGUI.Pages.Controls
{
    /// <summary>
    /// Interaction logic for KeyControl.xaml
    /// </summary>
    public partial class KeyControl : UserControl
    {
        public KeyControl()
        {
            InitializeComponent();
            Command = null;
            UpdateText();
        }



        public IKeystrokeCommand Command { get; set; }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="value"></param>
        public void UpdateCommand(IKeystrokeCommand value)
        {
            this.Command = value;
            UpdateCommand(value);
        }

        /// <summary>
        /// Updates the displayed text on this control.
        /// </summary>
        private void UpdateText()
        {
            if (Command == null)
            {
                textBlock.Text = "N/A";
                textBlock.Foreground = Brushes.Gray;
            }
            else textBlock.Foreground = Brushes.Black;

            if (Command is UnicodeCommand)
            {
                textBlock.Text = (Command as UnicodeCommand).ContentAsText;
            }
            else textBlock.Text = "...";
        }

        #region Exposing properties

        [Browsable(false), Description("Text content of this key; " +
            "Unicode commands will display their codepoints as text. " +
            "Other types of command may display an icon or symbol.")]
        public string Text
        {
            get { return textBlock.Text; }
            private set { textBlock.Text = value; } // normally set only by UpdateText
        }

        public double FontSize
        {
            get { return textBlock.FontSize; }
            set { textBlock.FontSize = value; }
        }

        public FontFamily FontFamily
        {
            get { return textBlock.FontFamily; }
            set { textBlock.FontFamily = value; }
        }

        public Brush ForegroundBrush
        {
            get { return textBlock.Foreground; }
            set { textBlock.Foreground = value; border.BorderBrush = value; }
        }

        #endregion

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new UnicodeCommandDialog();
            if (dialog.ShowDialog() ?? false)
            {
                Command = dialog.Command;
                UpdateText();
            }
        }
    }
}

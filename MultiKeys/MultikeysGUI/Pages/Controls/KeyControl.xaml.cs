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
using System.Globalization;

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
            Scancode = null;
            UpdateText();
        }


        
        public IKeystrokeCommand Command { get; private set; }

        
        public class ScancodeConverter : TypeConverter
        {
            public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
            {
                if (sourceType == typeof(string)) return true;
                else return base.CanConvertFrom(context, sourceType);
            }
            public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
            {
                if (destinationType == typeof(string)) return true;
                else return base.CanConvertTo(context, destinationType);
            }
            public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
            {
                if (!(value is string)) return null;
                return new Scancode(value as string);
            }
            public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
            {
                if (!(value is Scancode))
                    return null;
                if (destinationType != typeof(string))
                    return base.ConvertTo(context, culture, value, destinationType);
                return (value as Scancode).ToString();
            }
        }
        /// <summary>
        /// This property is not used internally, but provides a useful id for this control.
        /// </summary>
        [TypeConverter(typeof(ScancodeConverter))]
        public Scancode Scancode { get; set; }
        

        /// <summary>
        /// 
        /// </summary>
        /// <param name="value"></param>
        public void UpdateCommand(IKeystrokeCommand value)
        {
            this.Command = value;
            UpdateText();
        }

        /// <summary>
        /// Updates the displayed text on this control.
        /// </summary>
        private void UpdateText()
        {
            if (Command == null)
            {
                MiddleLabel.Text = "N/A";
                MiddleLabel.Foreground = Brushes.Gray;
            }
            else MiddleLabel.Foreground = Brushes.Black;

            if (Command is UnicodeCommand)
            {
                MiddleLabel.Text = (Command as UnicodeCommand).ContentAsText;
            }
            else MiddleLabel.Text = "...";
        }

        #region Exposing properties

        [Browsable(false), Description("Text content of this key; " +
            "Unicode commands will display their codepoints as text. " +
            "Other types of command may display an icon or symbol.")]
        public string Text
        {
            get { return MiddleLabel.Text; }
            private set { MiddleLabel.Text = value; } // normally set only by UpdateText
        }

        public double FontSize
        {
            get { return MiddleLabel.FontSize; }
            set { MiddleLabel.FontSize = value; }
        }

        public FontFamily FontFamily
        {
            get { return MiddleLabel.FontFamily; }
            set { MiddleLabel.FontFamily = value; }
        }

        public Brush ForegroundBrush
        {
            get { return MiddleLabel.Foreground; }
            set { MiddleLabel.Foreground = value; border.BorderBrush = value; }
        }

        public string BottomLabelText
        {
            get { return BottomLabel.Text; }
            set { BottomLabel.Text = value; }
        }

        #endregion

        #region Custom Events

        public event EventHandler KeyClicked;
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            KeyClicked?.Invoke(this, e);        // <- this control raises the KeyClicked event
        }

        #endregion
        
    }
    
}

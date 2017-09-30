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
using MultikeysGUI.View.Dialogues;
using System.Globalization;
using MultikeysGUI.Domain.Layout;

namespace MultikeysGUI.View.Controls
{
    /// <summary>
    /// Interaction logic for KeyControl.xaml
    /// </summary>
    public partial class KeyControl : UserControl
    {
        /// <summary>
        /// Although this constructor initializes certain properties,
        /// the Shape property must be called in order to determine which shape of key to draw.
        /// </summary>
        public KeyControl()
        {
            InitializeComponent();

            // Create graphical elements based on key shape

            Command = null;
            Scancode = null;
            UpdateText();
        }

        /// <summary>
        /// Draws the borders according to the shape.
        /// </summary>
        /// <param name="shape">Shape to be drawn; some shapes require specific dimensions.</param>
        private void DrawShape(PhysicalKeyShape shape)
        {
            // Choose the correct resource from this control's resource dictionary
            KeyContainerGrid.Children.Clear();

            switch (shape)
            {
                case PhysicalKeyShape.Rectangular:
                    KeyContainerGrid.Children.Add(this.Resources["RectangularKey"] as Grid);
                    break;
                case PhysicalKeyShape.LShapedReturn:
                    KeyContainerGrid.Children.Add(this.Resources["ReturnKey"] as Grid);
                    break;
                case PhysicalKeyShape.BigReturn:
                    KeyContainerGrid.Children.Add(this.Resources["BigReturnKey"] as Grid);
                    break;
            }

        }
        

        private PhysicalKeyShape _shape;
        public PhysicalKeyShape Shape
        {
            get { return _shape; }
            set { _shape = value; DrawShape(_shape); }
        }


        public IKeystrokeCommand Command { get; private set; }

        /// <summary>
        /// This converter is necessary for exposing a property of type Scancode that can be set from
        /// xaml as a string.
        /// </summary>
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
        /// This is a separate method from UpdateCommand, mostly
        /// so that it can be called in the constructor.
        /// </summary>
        private void UpdateText()
        {
            if (Command == null)
            {
                MiddleLabel.Text = " ";
                MiddleLabel.Foreground = Brushes.Gray;
            }
            else
            {
                MiddleLabel.Foreground = Brushes.Black;

                if (Command is UnicodeCommand)
                {
                    MiddleLabel.Text = (Command as UnicodeCommand).ContentAsText;
                }
                else MiddleLabel.Text = "...";
            }
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

        public new double FontSize
        {
            get { return MiddleLabel.FontSize; }
            set { MiddleLabel.FontSize = value; }
        }

        public new FontFamily FontFamily
        {
            get { return MiddleLabel.FontFamily; }
            set { MiddleLabel.FontFamily = value; }
        }

        public Brush ForegroundBrush
        {
            get { return MiddleLabel.Foreground; }
            set { MiddleLabel.Foreground = value; }
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

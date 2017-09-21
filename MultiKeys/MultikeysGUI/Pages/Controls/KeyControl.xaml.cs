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
using MultikeysGUI.Layout;

namespace MultikeysGUI.Pages.Controls
{
    /// <summary>
    /// Interaction logic for KeyControl.xaml
    /// </summary>
    public partial class KeyControl : UserControl
    {
        public KeyControl(PhysicalKeyShape shape = PhysicalKeyShape.Rectangular)
        {
            InitializeComponent();

            // Create graphical elements based on key shape
            

            Command = null;
            Scancode = null;
            UpdateText();

            /*
             <Border x:Name="border" BorderBrush="#FF000000" BorderThickness="3" CornerRadius="5" Margin="1" Panel.ZIndex="-1"/>
             */
        }


        /// <summary>
        /// Draws the borders according to the shape.
        /// </summary>
        /// <param name="shape"></param>
        private void DrawShape(PhysicalKeyShape shape)
        {
            BorderGrid.Children.Clear();

            if (shape == PhysicalKeyShape.Rectangular)
            {
                // Build one simple rectangle that covers the whole rectangular control
                var border = new Border
                {
                    BorderBrush = Brushes.Black,
                    BorderThickness = new Thickness(3),
                    CornerRadius = new CornerRadius(5),
                    Margin = new Thickness(1)
                };
                Panel.SetZIndex(border, -1);
                Grid.SetRowSpan(border, 2);
                Grid.SetColumnSpan(border, 2);
                BorderGrid.Children.Add(border);
            }
            else if (shape == PhysicalKeyShape.StandardEnter)
            {
                // Build a top-left border that covers the top half
                var tlBorder = new Border
                {
                    BorderBrush = Brushes.Black,
                    BorderThickness = new Thickness(3, 3, 0, 0),
                    CornerRadius = new CornerRadius(5),
                    Margin = new Thickness(1)
                };
                Panel.SetZIndex(tlBorder, -1);
                Grid.SetColumnSpan(tlBorder, 2);
                BorderGrid.Children.Add(tlBorder);

            }
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

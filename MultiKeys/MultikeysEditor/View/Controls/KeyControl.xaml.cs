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

using MultikeysEditor.Model;
using System.ComponentModel;
using MultikeysEditor.View.Dialogues;
using System.Globalization;
using MultikeysEditor.Domain.Layout;

namespace MultikeysEditor.View.Controls
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
            Refresh();
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


        // If both Command and Modifier are null at the same time, then this key is not remapped.

        /// <summary>
        /// Represents the command mapped to this key.
        /// Note that this cannot be non-null at the same time as Modifier is non-null.
        /// </summary>
        public IKeystrokeCommand Command { get; private set; }

        /// <summary>
        /// Represents the modifier mapped to this key.
        /// Note that this cannot be non-null at the same time as Command is non-null.
        /// </summary>
        public Modifier Modifier { get; private set; }

        /// <summary>
        /// Whether or not this modifier key is selected. Has no effect if Modifier is null.
        /// </summary>
        public bool IsModifierSelected { get; set; }



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
        /// Tells this control to render the informed command.
        /// The command will be remembered. If this key was previously a modifier,
        /// then it is changed into a command key.
        /// </summary>
        /// <param name="value"></param>
        public void SetCommand(IKeystrokeCommand value)
        {
            this.Command = value;
            if (this.Modifier != null) this.Modifier = null;
            Refresh();
        }

        /// <summary>
        /// Tells this control to render as the informed modifier.
        /// If this key was previously a command, then it is changed into a modifier key.
        /// </summary>
        /// <param name="value"></param>
        public void SetModifier(Modifier value)
        {
            this.Modifier = value;
            if (this.Command != null) this.Command = null;
            Refresh();
        }

        private void SetOutlineColor(Brush color)
        {
            // Change the stroke brush of every rectangle in KeyContainerGrid
            foreach (var element in KeyContainerGrid.Children)
            {
                if (element is Grid)
                {
                    foreach (var child in (element as Grid).Children)
                    {
                        if (child is Rectangle) try
                            {
                                (child as Rectangle).Stroke = color;
                            }
                            catch (Exception) { }
                    }
                    return;
                }
            }
        }

        /// <summary>
        /// Updates the displayed text on this control.
        /// This is a separate method from UpdateCommand, mostly
        /// so that it can be called in the constructor.
        /// </summary>
        private void Refresh()
        {
            // Check for possible states. For each of them, set:
            //          The label that appears on this key
            //          The text's color
            //          The key's outline color

            // 1. This key is not remapped
            if (Command == null && Modifier == null)
            {
                MiddleLabel.Text = " ";
                MiddleLabel.Foreground = Brushes.Gray;
                SetOutlineColor(Brushes.SlateGray);
            }
            // 2. This key is remapped to a command
            else if (Command != null)
            {
                MiddleLabel.Foreground = Brushes.Black;

                if (Command is DeadKeyCommand)  // dead keys show up as red characters
                {
                    MiddleLabel.Text = (Command as UnicodeCommand).ContentAsText;
                    MiddleLabel.Foreground = Brushes.Red;
                }
                else if (Command is UnicodeCommand)
                {
                    MiddleLabel.Text = (Command as UnicodeCommand).ContentAsText;
                    MiddleLabel.Foreground = Brushes.Black;
                }
                else MiddleLabel.Text = "...";

                SetOutlineColor(Brushes.Black);
            }
            // 3. This key is remapped to a modifier, and that modifier is not pressed down
            else if (Modifier != null && !IsModifierSelected)
            {
                MiddleLabel.Text = Modifier.Name;
                MiddleLabel.Foreground = Brushes.Blue;
                SetOutlineColor(Brushes.Black);
            }
            // 4. This key is remapped to a modifier, and that modifier is pressed down
            else if (Modifier != null && IsModifierSelected)
            {
                MiddleLabel.Text = Modifier.Name;
                MiddleLabel.Foreground = Brushes.Blue;
                SetOutlineColor(Brushes.Blue);
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
            // Raises the key clicked event, with itself as sender.
            // It's up to its parent to handle what happens afterwards, such as
            // changing the content of this key.
            KeyClicked?.Invoke(this, e);
        }

        #endregion
        
    }
    
}

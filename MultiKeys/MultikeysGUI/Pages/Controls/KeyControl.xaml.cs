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

            // Draw shape
        }

        /// <summary>
        /// Draws the borders according to the shape.
        /// </summary>
        /// <param name="shape"></param>
        private void DrawShape(PhysicalKeyShape shape)
        {

            if (shape == PhysicalKeyShape.Rectangular)
            {
                BorderGrid.Children.Clear();


                // Build one simple rectangle that covers the whole rectangular control
                var rectangle = MakeRectangle();
                Panel.SetZIndex(rectangle, -1);
                BorderGrid.Children.Add(rectangle);
            }
            else if (shape == PhysicalKeyShape.StandardEnter)
            {
                

                

                /*
             <Grid.RowDefinitions>
                    <RowDefinition Height="*" />
                    <RowDefinition Height="*" />
                    <RowDefinition Height="*" />
                    <RowDefinition Height="*" />
                </Grid.RowDefinitions>
                <Grid.ColumnDefinitions>
                    <ColumnDefinition Width="15*"/>
                    <ColumnDefinition Width="5*"/>
                    <ColumnDefinition Width="50*"/>
                    <ColumnDefinition Width="50*"/>
                </Grid.ColumnDefinitions>

                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.RowSpan="2" Width="1000"   Panel.ZIndex="-7" />
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.ColumnSpan="4" Height="1000" Margin="0, 0, 1.8, 0"   Panel.ZIndex="-6"/>
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.Column="3" Grid.RowSpan="4" Width="1000" HorizontalAlignment="Right"  Margin="0, 0, 1.5, 0" Panel.ZIndex="-5"/>
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.ColumnSpan="2" Grid.Column="2" Grid.Row="3" VerticalAlignment="Bottom" Height="1000" Margin="0, 0, 1.5, 0" Panel.ZIndex="-4"/>
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.Column="1" Grid.Row="2" Margin="-3" VerticalAlignment="Top" HorizontalAlignment="Right" Height="1000" Width="1000" Panel.ZIndex="-3" />
                
                <!-- The following two exist to smoothen out the line thickness -->
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.RowSpan="2" Width="1000"   Panel.ZIndex="-7" />
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.Column="1" Grid.Row="2" Margin="-3.49" VerticalAlignment="Top" HorizontalAlignment="Right" Height="1000" Width="1000" Panel.ZIndex="-3" />
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.Column="2" HorizontalAlignment="Center" Height="1000" Width="1000"  Panel.ZIndex="-1" />
                <Rectangle Stroke="Black" StrokeThickness="3" RadiusX="4" RadiusY="4" Grid.Column="2" Grid.Row="3" VerticalAlignment="Bottom" Height="1000" Width="1000"  Panel.ZIndex="-1" />
            </Grid>    
             */
            }
        }
        /// <summary>
        /// For use in DrawShape()
        /// </summary>
        private Rectangle MakeRectangle()
        {
            return new Rectangle
            {
                Fill = Brushes.Transparent,
                Stroke = Brushes.Black,
                StrokeThickness = 3,
                StrokeLineJoin = PenLineJoin.Round,
                RadiusX = 4,
                RadiusY = 4,
            };
        }

        /// <summary>
        /// This holds actions that should be done during rendering.
        /// Good for clipping, which requires the elements to exist on screen in order to clip.
        /// </summary>
        private ICollection<Action> doOnRender = new List<Action>();
        protected override void OnRender(DrawingContext drawingContext)
        {
            foreach (var action in doOnRender)
                action.Invoke();
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

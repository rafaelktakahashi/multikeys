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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MultikeysGUI.Pages.Controls
{
    /// <summary>
    /// Interaction logic for LayerControl.xaml
    /// </summary>
    public partial class LayerControl : UserControl
    {
        public LayerControl()
        {
            InitializeComponent();

            // Setting the unit lenght used for each key control.
            UnitLenght = 45;

            // Add every key into this control.
            // Every coordinate is calculated.
            GenerateANSIKeys();
        }

        /// <summary>
        /// This property describes the base unit for each key.
        /// </summary>
        public double UnitLenght { get; }

        /// <summary>
        /// Factory method for creating new key controls.
        /// </summary>
        /// <param name="scancode">Scancode to </param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="xOffset"></param>
        /// <param name="yOffset"></param>
        /// <returns></returns>
        private KeyControl MakeKey(string scancode, double width, double height, double xOffset, double yOffset)
        {
            return new KeyControl
            {
                Scancode = Scancode.FromString(scancode),
                Width = width * UnitLenght,
                Height = height * UnitLenght,
                VerticalAlignment = VerticalAlignment.Top,
                HorizontalAlignment = HorizontalAlignment.Left,
                Margin = new Thickness(xOffset * UnitLenght + 10, yOffset * UnitLenght + 10, 0, 0)
            };
        }

        private void GenerateANSIKeys()
        {
            // Function keys

        }

        /// <summary>
        /// Alias of this layer given by the user.
        /// </summary>
        public string Alias { get; set; }

        /// <summary>
        /// The mapping between scancodes and keystroke commands.
        /// Setting or changing this property does not refresh the view.
        /// <para/>
        /// Scancodes present in this control but not in Layout are considered not remapped.
        /// <para/>
        /// This property may be set to null in case of a non-remapped layer.
        /// </summary>
        public IDictionary<Scancode, IKeystrokeCommand> Layout { get; set; }


        /// <summary>
        /// Refreshes every key in this view.
        /// </summary>
        public void RefreshView()
        {
            ;
        }

    }

}

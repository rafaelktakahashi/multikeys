using MultikeysGUI.Layout;
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
        class ScancodeComparer : IEqualityComparer<Scancode>
        {
            public bool Equals(Scancode x, Scancode y)
            {
                return x == y;  // operator is defined for Scancode
            }

            public int GetHashCode(Scancode obj)
            {
                return obj.MakeCode;
            }
        }

        public LayerControl()
        {
            InitializeComponent();

            Layout = new Dictionary<Scancode, IKeystrokeCommand>(new ScancodeComparer());
            KeyControls = new Dictionary<Scancode, KeyControl>(new ScancodeComparer());
        }

        /// <summary>
        /// Creates and renders the collection of keys on the screen.<para/>
        /// This method must be called at least once on this control.<para/>
        /// If called again after it's already loaded, this method will not overwrite
        /// any existing data, but certain keys may become inaccessible if the layout is changed.<para/>
        /// Do not call this method to refresh the view; use <code>RefreshView</code> instead.
        /// </summary>
        /// <param name="layout">Physical layout representing the scancode, size and position of each physical key.</param>
        /// <param name="unitLength">Lenght in px used as the unit length.
        /// The resulting keyboard is always 23 units wide and 6.5 units high.</param>
        public void LoadLayout(IPhysicalLayout layout, double unitLength = 45)
        {
            KeyControls.Clear();
            foreach (var key in layout.Keys)
            {
                // Make a key using the provided information.
                var newKey = MakeKey(key, unitLength);
                // Add the new key to this control. It will be rendered.
                KeysGrid.Children.Add(newKey);
                // Add a handler so that each key fires an event.
                newKey.KeyClicked += new EventHandler(OnAnyKeyControlClicked);
                // Also add the control to the dictionary so that it can be retrieved later
                KeyControls.Add(newKey.Scancode, newKey);
            }
        }
        
        /// <summary>
        /// Make a new KeyControl that's ready to be rendered, based on the information provided.
        /// </summary>
        /// <param name="key">Object representing scancode, size and position of a physical key.</param>
        /// <param name="unitLength">Length, in pixels, to be used as the unit length for size and position.</param>
        /// <returns></returns>
        private KeyControl MakeKey(PhysicalKey key, double unitLength)
        {
            return new KeyControl
            {
                Scancode = key.Scancode,
                Width = key.Width * unitLength,
                Height = key.Height * unitLength,
                VerticalAlignment = VerticalAlignment.Top,
                HorizontalAlignment = HorizontalAlignment.Left,     // its position in the control is set through Margin
                Margin = new Thickness(key.XOffset * unitLength, key.YOffset * unitLength, 0, 0),
                // Set shape when implemented
            };
        }

        /// <summary>
        /// Alias of this layer given by the user.
        /// </summary>
        public string Alias { get; set; }

        /// <summary>
        /// The mapping between scancodes and keystroke commands.<para/>
        /// Setting or changing this property does not refresh the view. For that, call RefreshView.
        /// <para/>
        /// Scancodes present in this control but not in Layout are considered not remapped.
        /// <para/>
        /// This property may be set to null in case of a non-remapped layer.
        /// </summary>
        public IDictionary<Scancode, IKeystrokeCommand> Layout { get; set; }

        /// <summary>
        /// Private dictionary for all KeyControls.
        /// </summary>
        private IDictionary<Scancode, KeyControl> KeyControls { get; set; }


        /// <summary>
        /// Refreshes every key in this view according to the commands in Layout.<para/>
        /// Call this function after setting the Layout.
        /// </summary>
        public void RefreshView()
        {
            // This algorithm doesn't look as fast as it could be.
            foreach (var controlPair in KeyControls)
            {
                // get its corresponding command
                if (Layout.ContainsKey(controlPair.Key))
                {
                    KeyControls[controlPair.Key].UpdateCommand(Layout[controlPair.Key]);
                }
                else
                {
                    KeyControls[controlPair.Key].UpdateCommand(null);
                }
            }
        }

        #region Events

        public event EventHandler KeyClicked;

        // bubble up the event
        private void OnAnyKeyControlClicked(object sender, EventArgs e)
        {
            KeyClicked?.Invoke(sender, e);
        }

        #endregion

    }

}

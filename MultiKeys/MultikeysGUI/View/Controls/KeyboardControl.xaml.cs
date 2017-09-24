using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using MultikeysGUI.Model;
using MultikeysGUI.Domain.Layout;

namespace MultikeysGUI.View.Controls
{
    /// <summary>
    /// Event that is raised by a KeyboardControl when any key in it is pressed on screen.
    /// </summary>
    public class KeyClickedEventArgs : EventArgs
    {
        /// <summary>
        /// Identifies the keyboard where the key click came from.
        /// </summary>
        public Keyboard Keyboard { get; set; }
        /// <summary>
        /// Identifies the key that was clicked.
        /// </summary>
        public Scancode Scancode { get; set; }
        /// <summary>
        /// The command that the clicked key is mapped to.
        /// This may be null, in which case the clicked key is not remapped.
        /// </summary>
        public IKeystrokeCommand Command { get; set; }
    }

    /// <summary>
    /// When a key is clicked on this keyboard, an event containing information about
    /// the clicked key is sent.
    /// </summary>
    public delegate void KeyClickedDelegate(object sender, KeyClickedEventArgs args);

    /// <summary>
    /// Interaction logic for KeyboardControl.xaml
    /// </summary>
    public partial class KeyboardControl : UserControl
    {
        public KeyboardControl() : this(kb: null, physLayout: null)
        { }
        
        public KeyboardControl(Keyboard kb, IPhysicalLayout physLayout)
        {
            InitializeComponent();

            _keyboard = kb;
            Layer.RenderLayout(physLayout);
            Layer.RefreshView(kb.Layers[0].Commands);
        }

        // Fields

        private Keyboard _keyboard;   // not a property


        // Events
        public event KeyClickedDelegate KeyClicked;
        

        // Event handlers
        public void LayerKeyClicked(object sender, EventArgs e)
        {
            // Bubble up the event
            KeyControl kc = sender as KeyControl;
            KeyClicked?.Invoke(this, new KeyClickedEventArgs
            {
                Command = kc.Command,
                Keyboard = this._keyboard,
                Scancode = kc.Scancode,
            });
        }

    }
}

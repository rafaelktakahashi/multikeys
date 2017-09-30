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

            Keyboard = kb;
            Layer.RenderLayout(physLayout);
            Layer.RefreshView(kb.Layers[0].Commands);
        }

        /// <summary>
        /// This control represent this object's state.
        /// </summary>
        public Keyboard Keyboard { get; private set; }

        /// <summary>
        /// Currently selected layer that should be rendered on screen.
        /// This changes when the combination of pressed modifiers changes.
        /// </summary>
        private Layer _activeLayer;

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
                Keyboard = this.Keyboard,
                Scancode = kc.Scancode,
            });
        }

        public void ModifierSelectionChanged(object sender, EventArgs e)
        {
            // Get the current selection of modifiers
            IEnumerable<string> selectedModifierNames = ModifiersControl.SelectedModifiers;
            // Update the active layer according to the modifiers
            foreach (Layer layer in Keyboard.Layers)
            {
                if (layer.ModifierCombination.All(selectedModifierNames.Contains)
                    && layer.ModifierCombination.Count == selectedModifierNames.Count())
                {
                    // The current combination of modifiers matches a layer
                    _activeLayer = layer;
                    // Render it on screen
                    Layer.RefreshView(layer.Commands);
                }
                // if got here, there's no remapped layer.
            }

            // TODO: This code for checking layers should be moved into a lookup container that
            // returns null for invalid combinations and can be queried for all valid ones.
        }

    }
}

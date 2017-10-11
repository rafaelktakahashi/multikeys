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
using MultikeysGUI.View.Dialogues;

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
        public string KeyboardName { get; set; }
        /// <summary>
        /// Identifies the key that was clicked.
        /// </summary>
        public Scancode Scancode { get; set; }
        /// <summary>
        /// The command that the clicked key is mapped to.
        /// This may be null, in which case the clicked key is not remapped.
        /// This must be null if Modifier is non-null.
        /// </summary>
        public IKeystrokeCommand Command { get; set; }
        /// <summary>
        /// The modifier that this clicked key is registered as.
        /// This must be null if Command is non-null.
        /// </summary>
        public Modifier Modifier { get; set; }
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

            
            // Setup properties
            if (kb != null)
            {
                // being initialized from an existing keyboard
                UniqueName = kb.UniqueName;
                Alias = kb.Alias;
                _modifiers = kb.Modifiers;
                _layers = kb.Layers;
            }
            else
            {
                // being initialized as a brand new keyboard
                UniqueName = string.Empty;
                Alias = null;
                _modifiers = new List<Modifier>();
                _layers = new List<Layer>();
            }

            // Setup modifiers control
            ModifiersControl.InitializeModifiers(_modifiers);

            // Setup the layer to the default one (no modifiers)
            Layer.SetLayoutToRender(physLayout);
            Layer.RefreshView(kb.Layers[0].Commands, _modifiers);
            _activeLayer = kb.Layers[0];            // TODO: Using Layers[0] doesn't cut it.

            // Setup the selected key to none
            SelectedKey = null;

            // Show the keyboard's name
            LabelKeyboardName.Text = kb.UniqueName == "" ? Properties.Strings.KeyboardNameDialogAnyKeyboard : kb.UniqueName;
            // TODO: Show the keyboard's alias somewhere
        }

        /// <summary>
        /// This method extracts a Keyboard object from this control.
        /// </summary>
        /// <returns>An instance of Keyboard that contains this control's information</returns>
        public Keyboard GetKeyboard()
        {
            return new Keyboard()
            {
                UniqueName = UniqueName,
                Alias = Alias,
                Modifiers = _modifiers,         // May need redoing
                Layers = _layers
            };
        }

        #region Properties And Fields

        public string UniqueName { get; set; }
        public string Alias { get; set; }

        private IList<Modifier> _modifiers;
        private IList<Layer> _layers;

        // A readonly empty dictionary with 0 capacity that represents an empty layer.
        private readonly IDictionary<Scancode, IKeystrokeCommand> EmptyLayerCommands
            = new Dictionary<Scancode, IKeystrokeCommand>(0);

        /// <summary>
        /// Currently selected layer that should be rendered on screen.
        /// This changes when the combination of pressed modifiers changes.
        /// </summary>
        private Layer _activeLayer;

        /// <summary>
        /// Currently selected key, initialized as null.
        /// </summary>
        public KeyControl SelectedKey { get; private set; }

        #endregion


        /// <summary>
        /// This event occurs whenever a key is clicked.
        /// The parent control is responsible for displaying detailed information about the selected key.
        /// </summary>
        public event KeyClickedDelegate KeyClicked;

        // Event handlers
        public void LayerKeyClicked(object sender, EventArgs e)
        {
            var keyControl = sender as KeyControl;

            // If the pressed key was a modifier, then the modifier state must be updated
            // in this control, in the layer control and in the modifier control (not in the key control, that's the layer's problem)
            if (keyControl.Modifier != null)
            {
                bool newModifierState = !keyControl.IsModifierSelected;

                // update the modifier control
                ModifiersControl.UpdateModifierState(keyControl.Modifier, newModifierState);

                // update the layer control and rerender it
                Layer.RefreshView(keyControl.Modifier, newModifierState);
            }

            // Enable or diable buttons according to the newly selected item.
            // TODO

            // Update this control's selected key and command
            SelectedKey = keyControl;

            // Bubble up the event, as to transmit information
            KeyClicked?.Invoke(this, new KeyClickedEventArgs
            {
                Command = keyControl.Command,
                Modifier = keyControl.Modifier,
                KeyboardName = UniqueName,
                Scancode = keyControl.Scancode,
            });
        }

        private class ModifierComparerByName : IEqualityComparer<Modifier>
        {
            public bool Equals(Modifier x, Modifier y)
            {
                return x.Name.Equals(y.Name);
            }

            public int GetHashCode(Modifier obj)
            {
                return obj.Name.GetHashCode();
            }
        }
        /// <summary>
        /// Handler for the ModifierSelectionChanged event fired by the modifier control.
        /// </summary>
        public void ModifierSelectionChanged(object sender, EventArgs e)
        {
            // Get both the currently selected modifers and the currently unselected modifiers
            var selectedModifiers = ModifiersControl.SelectedModifiers;
            var unselectedModifiers = _modifiers.Except(selectedModifiers, new ModifierComparerByName());

            var selectedModNames =
                from x in selectedModifiers select x.Name;
            var unselectedModNames =
                from x in unselectedModifiers select x.Name;

            // Look for a layer that matches
            foreach (Layer layer in _layers)
            {
                // criteria for matching:
                // 1. Each currently selected modifier must exist in the layer's list of modifiers
                // 2. Each currently unselected modifier must not exist in the layer's list of modifiers
                if (selectedModNames.All(layer.ModifierCombination.Contains)
                    && !unselectedModNames.Any(layer.ModifierCombination.Contains))
                {
                    // found the correct layer
                    _activeLayer = layer;
                    // render it
                    Layer.RefreshView(layer.Commands, _modifiers);
                    // done
                    return;
                }
            }

            // No layer was found
            // render no command
            Layer.RefreshView(EmptyLayerCommands, _modifiers);
        }


        private void ButtonAssignCommand_Click(object sender, RoutedEventArgs e)
        {
            // If no key is selected:
            if (SelectedKey == null)
                return;     // TODO: Warn the user properly

            // If the selected key is already registered as modifier, then the operation
            // cannot continue (the button should have been disabled anyway)
            if (_modifiers.Any(mod => mod.Scancodes.Contains(SelectedKey.Scancode)))
            {
                return;
            }

            // Show a dialog, then check the result of its ShowDialog.
            var dialog = new KeystrokeCommandDialog();
            if (dialog.ShowDialog() == true)
            {
                // Retrieve the new command from the dialog:
                var newCommand = dialog.Command;
                // Give it to the key:
                SelectedKey.SetCommand(newCommand);
                // Also update the current layer:
                UpdateCurrentLayerWithNewCommand(SelectedKey.Scancode, newCommand);
            }
        }

        private void ButtonEditCommand_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ButtonRemoveCommand_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ButtonRegisterModifier_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ButtonUnregisterModifier_Click(object sender, RoutedEventArgs e)
        {

        }


        private void UpdateCurrentLayerWithNewCommand(Scancode sc, IKeystrokeCommand command)
        {
            // TODO: Add special case for an empty layer, which is represented by a dummy dictionary
            if (_activeLayer.Commands.ContainsKey(sc))
            {
                _activeLayer.Commands[sc] = command;
            }
            else
            {
                _activeLayer.Commands.Add(sc, command);
            }
        }

        /// <summary>
        /// Called when the user wants to change the keyboard's unique name.
        /// Should detect a physical device's name, then set it on this keyboard.
        /// </summary>
        private void ButtonKeyboardNameEdit_Click(object sender, RoutedEventArgs e)
        {
            var kbNameDialog = new KeyboardNameDialog(UniqueName);
            if (kbNameDialog.ShowDialog() == true)
            {
                UniqueName = kbNameDialog.KeyboardName;
                LabelKeyboardName.Text = UniqueName;
            }
        }
    }
}

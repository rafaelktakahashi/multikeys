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
                _layers = kb.Layers;
            }
            else
            {
                // being initialized as a brand new keyboard
                UniqueName = string.Empty;
                Alias = null;
                _layers = new List<Layer>();
            }

            // Setup modifiers control
            ModifiersControl.InitializeModifiers(kb.Modifiers);

            // Setup the layer to the default one (no modifiers)
            Layer.SetLayoutToRender(physLayout);
            Layer.RefreshView(kb.Layers[0].Commands, ModifiersControl.Modifiers);
            _activeLayer = kb.Layers[0];            // TODO: Using Layers[0] doesn't cut it.

            // Setup the selected key to none
            SelectedKey = null;

            // Show the keyboard's name
            LabelKeyboardName.Text = kb.UniqueName == "" ? Properties.Strings.KeyboardNameDialogAnyKeyboard : kb.UniqueName;
            // TODO: Show the keyboard's alias somewhere

            UpdateButtonStates();
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
                Modifiers = ModifiersControl.Modifiers,
                Layers = _layers
            };
        }

        #region Properties And Fields

        public string UniqueName { get; set; }
        public string Alias { get; set; }
        
        // modifiers are a responsibility of the ModifierControl
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
            
            // Update this control's selected key and command
            SelectedKey = keyControl;

            // Enable or diable buttons according to the newly selected item.
            UpdateButtonStates();

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
        private ModifierComparerByName modifierComparerByName = new ModifierComparerByName();

        /// <summary>
        /// Handler for the ModifierSelectionChanged event fired by the modifier control.
        /// </summary>
        public void ModifierSelectionChanged(object sender, EventArgs e)
        {
            // Get both the currently selected modifers and the currently unselected modifiers
            var selectedModifiers = ModifiersControl.SelectedModifiers;
            var unselectedModifiers = ModifiersControl.Modifiers.Except(selectedModifiers, modifierComparerByName);

            var selectedModNames =
                from x in selectedModifiers select x.Name;
            var unselectedModNames =
                from x in unselectedModifiers select x.Name;

            bool layerFound = false;
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
                    Layer.RefreshView(layer.Commands, ModifiersControl.Modifiers);
                    // done
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                // No layer was found
                // render no command
                Layer.RefreshView(EmptyLayerCommands, ModifiersControl.Modifiers);
                // Active layer becomes null, to signify an empty layer
                _activeLayer = null;
            }

            // Note that emtpy layers are nulls to save space; the number of potential layers is the factorial
            // of the number of modifiers, so that could skyrocket fast.

            // Change the visuals of the keys on screen to reflect the new selection
            foreach (var mod in selectedModifiers)
                Layer.RefreshView(mod, true);
            foreach (var mod in unselectedModifiers)
                Layer.RefreshView(mod, false);
            // Explanation: The user can change layers both by pressing the buttons in the modifiers control, and
            // by pressing the keys on the layer control. In each case, the other control should be updated to match
            // the current state; this method takes care of the first case (modifiers control that updates layer control)
        }
        
        


        private void UpdateButtonStates()
        {
            // first off, if no key is selected, then every button is disabled
            if (SelectedKey == null)
            {
                ButtonAssignCommand.IsEnabled = false;
                ButtonEditCommand.IsEnabled = false;
                ButtonRemoveCommand.IsEnabled = false;
                ButtonRegisterModifier.IsEnabled = false;
                ButtonUnregisterModifier.IsEnabled = false;

                return;
            }

            // new command
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                ButtonAssignCommand.IsEnabled = false;
            else if (SelectedKey.Command != null)
                ButtonAssignCommand.IsEnabled = false;
            else
                ButtonAssignCommand.IsEnabled = true;

            // edit command
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                ButtonEditCommand.IsEnabled = false;
            else if (SelectedKey.Command == null)
                ButtonEditCommand.IsEnabled = false;
            else
                ButtonEditCommand.IsEnabled = true;

            // remove command
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                ButtonRemoveCommand.IsEnabled = false;
            else if (SelectedKey.Command == null)
                ButtonRemoveCommand.IsEnabled = false;
            else
                ButtonRemoveCommand.IsEnabled = true;

            // register modifier
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                ButtonRegisterModifier.IsEnabled = false;
            else if (SelectedKey.Command != null)
                ButtonRegisterModifier.IsEnabled = false;
            else ButtonRegisterModifier.IsEnabled = true;

            // unregister modifier
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                ButtonUnregisterModifier.IsEnabled = true;
            else
                ButtonUnregisterModifier.IsEnabled = false;

        }


        private void ButtonAssignCommand_Click(object sender, RoutedEventArgs e)
        {
            // If no key is selected:
            if (SelectedKey == null)
                return;

            // If the selected key is already registered as modifier, then the operation
            // cannot continue (the button should have been disabled anyway)
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                return;

            // If the key already has a command in it, the operation cannot continue
            if (SelectedKey.Command != null)
                return;

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
            if (SelectedKey == null)
                return;

            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                return;

            if (SelectedKey.Command == null)
                return;

            // Show a dialog, set it to edit mode.
            var dialog = new KeystrokeCommandDialog();
            dialog.UseExistingCommand(SelectedKey.Command);
            if (dialog.ShowDialog() == true)
            {
                var newCommand = dialog.Command;
                SelectedKey.SetCommand(newCommand);
                UpdateCurrentLayerWithNewCommand(SelectedKey.Scancode, newCommand);
            }
        }

        private void ButtonRemoveCommand_Click(object sender, RoutedEventArgs e)
        {
            if (SelectedKey == null)
                return;

            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                return;

            if (SelectedKey.Command == null)
                return;

            // Ask for confirmation, then remove the command
            MessageBoxResult answer = MessageBox.Show(
                Properties.Strings.WarningRemoveCommand,
                Properties.Strings.Confirmation,
                MessageBoxButton.YesNo,
                MessageBoxImage.Warning);

            if (answer == MessageBoxResult.Yes)
            {
                SelectedKey.SetCommand(null);
                _activeLayer.Commands.Remove(SelectedKey.Scancode);
            }
        }

        private void ButtonRegisterModifier_Click(object sender, RoutedEventArgs e)
        {
            if (SelectedKey == null)
                return;

            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                return;

            // Loop through all exiting layers; if any of them include a command for this scancode,
            // warn the user that registering this key will override those commands.
            foreach (var layer in _layers)
            {
                if (layer.Commands.ContainsKey(SelectedKey.Scancode))
                {
                    MessageBoxResult answer =
                        MessageBox.Show(
                            Properties.Strings.WarningModifierOverridesCommands,
                            Properties.Strings.Warning,
                            MessageBoxButton.YesNo,
                            MessageBoxImage.Warning);

                    if (answer == MessageBoxResult.Yes)
                        break;  // move on to rest of method
                    else
                        return; // interrupt operation
                }
            }

            var dialog = new ModifierDialog(ModifiersControl.Modifiers);
            if (dialog.ShowDialog() == true)
            {
                var newModifierName = dialog.ModifierName;
                var scancode = SelectedKey.Scancode;

                var newModifier = ModifiersControl.RegisterModifier(scancode, newModifierName);
                SelectedKey.SetModifier(newModifier);
            }
        }

        private void ButtonUnregisterModifier_Click(object sender, RoutedEventArgs e)
        {
            if (SelectedKey == null)
                return;

            if (!ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                return;

            // Ask for confirmation, then unregister the key
            MessageBoxResult answer = MessageBox.Show(
                Properties.Strings.WarningRemoveModifier,
                Properties.Strings.Confirmation,
                MessageBoxButton.YesNo,
                MessageBoxImage.Warning);

            if (answer == MessageBoxResult.Yes)
            {
                SelectedKey.SetModifier(null);
                ModifiersControl.RemoveModifier(SelectedKey.Scancode);
            }
        }


        /// <summary>
        /// Sets the new command on the currently active layer.
        /// If the currently active is empty, it is created.
        /// </summary>
        /// <param name="sc"></param>
        /// <param name="command"></param>
        private void UpdateCurrentLayerWithNewCommand(Scancode sc, IKeystrokeCommand command)
        {
            // Special handling for nonexistent layer:
            if (_activeLayer == null)
            {
                // create this layer
                _activeLayer = new Layer()
                {
                    ModifierCombination = ModifiersControl.SelectedModifiers.Select(it => it.Name).ToList()
                };

                _layers.Add(_activeLayer);
            }


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

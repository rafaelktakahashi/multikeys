using MultikeysEditor.Domain;
using MultikeysEditor.Domain.Layout;
using MultikeysEditor.Model;
using MultikeysEditor.View.Dialogues;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace MultikeysEditor.View.Controls
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
        /// <summary>
        /// The key control that was clicked to generate this event.
        /// </summary>
        public KeyControl KeyControl { get; set; }
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
        // No default constructor

        public KeyboardControl(Keyboard kb)
        {
            InitializeComponent();

            
            // Setup properties
            if (kb != null)
            {
                // being initialized from an existing keyboard
                UniqueName = kb.UniqueName;
                Alias = kb.Alias;
                _Layers = kb.Layers;
                PhysicalLayout = kb.PhysicalLayout;
                LogicalLayout = kb.LogicalLayout ?? "US"; // use this as default
            }
            else
            {
                // being initialized as a brand new keyboard
                UniqueName = string.Empty;
                Alias = Properties.Strings.NewKeyboardAlias;
                _Layers = new List<Layer>();
                PhysicalLayout = PhysicalLayoutStandard.ISO; // use this as default
                LogicalLayout = "US"; // use this as default, could be better to look at current layout
            }

            // Setup modifiers control
            ModifiersControl.InitializeModifiers(kb.Modifiers);

            // Setup the layer to the default one (no modifiers)
            Layer.SetLayoutToRender(new DomainFacade().GetPhysicalLayout(PhysicalLayout));
            Layer.Labels = LogicalLayoutFactory.GetLogicalLayout(LogicalLayout);
            Layer.RefreshView(kb.Layers[0].Commands, ModifiersControl.Modifiers);
            _activeLayer = kb.Layers[0];            // TODO: Using Layers[0] doesn't cut it.

            // Set the selected key to none
            SelectedKey = null;

            // Show the keyboard's name and alias
            LabelKeyboardName.Text = kb.UniqueName == "" ? Properties.Strings.KeyboardNameDialogAnyKeyboard : kb.UniqueName;
            if (string.IsNullOrEmpty(kb.Alias))
            {
                kb.Alias = Properties.Strings.NewKeyboardAlias;
            }
            LabelKeyboardAlias.Text = kb.Alias;

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
                Layers = _Layers,
                PhysicalLayout = PhysicalLayout,
                LogicalLayout = LogicalLayout,
            };
        }

        #region Properties And Fields

        public string UniqueName { get; set; }
        public string Alias { get; set; }
        
        // modifiers are a responsibility of the ModifierControl
        private IList<Layer> _Layers;

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

        private bool _EditingAlias;

        /// <summary>
        /// Physical layout to display this keyboard with.
        /// This is sent to the layout control, but it's kept here to be persisted.
        /// </summary>
        public PhysicalLayoutStandard PhysicalLayout { get; set; }
        /// <summary>
        /// Logical layout to display this keyboard with.
        /// This is sent to the layout control, but it's kept here to be persisted.
        /// </summary>
        public string LogicalLayout { get; set; }

        #endregion


        /// <summary>
        /// This event occurs whenever a key is clicked.
        /// The parent control is responsible for displaying detailed information about the selected key.
        /// </summary>
        public event KeyClickedDelegate KeyClicked;

        /// <summary>
        /// This event signifies that this object is requesting to be deleted.
        /// It should be handled by whatever control is responsible for this object.
        /// </summary>
        public event EventHandler KeyboardDeletionRequest;

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
                KeyControl = keyControl,
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
        private readonly ModifierComparerByName modifierComparerByName = new ModifierComparerByName();

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
            foreach (Layer layer in _Layers)
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
                ButtonChangeModifierName.IsEnabled = false;
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

            // change modifier name
            if (ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                ButtonChangeModifierName.IsEnabled = true;
            else
                ButtonChangeModifierName.IsEnabled = false;

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
                // Update the buttons
                UpdateButtonStates();
                // And emit a new click event with the new information to update other components
                KeyClicked?.Invoke(this, new KeyClickedEventArgs()
                {
                    Command = newCommand,
                    KeyboardName = UniqueName,
                    Modifier = SelectedKey.Modifier,
                    Scancode = SelectedKey.Scancode,
                    KeyControl = SelectedKey,
                });
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
                // Update the buttons
                UpdateButtonStates();
                // And emit a new click event with the new information to update other components
                KeyClicked?.Invoke(this, new KeyClickedEventArgs()
                {
                    Command = newCommand,
                    KeyboardName = UniqueName,
                    Modifier = SelectedKey.Modifier,
                    Scancode = SelectedKey.Scancode,
                    KeyControl = SelectedKey
                });
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
                // Update the buttons
                UpdateButtonStates();
                // And emit a new clicked event with the new information to update other components
                KeyClicked?.Invoke(this, new KeyClickedEventArgs()
                {
                    Command = null,
                    KeyboardName = UniqueName,
                    Modifier = SelectedKey.Modifier,
                    Scancode = SelectedKey.Scancode,
                    KeyControl = SelectedKey,
                });
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
            foreach (var layer in _Layers)
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
                // Update
                UpdateButtonStates();
                KeyClicked?.Invoke(this, new KeyClickedEventArgs()
                {
                    Command = SelectedKey.Command,
                    KeyboardName = UniqueName,
                    Modifier = newModifier,
                    Scancode = SelectedKey.Scancode,
                    KeyControl = SelectedKey,
                });
            }
        }

        private void ButtonChangeModifierName_Click(object sender, RoutedEventArgs e)
        {
            if (SelectedKey == null)
                return;

            if (!ModifiersControl.IsRegisteredAsModifier(SelectedKey.Scancode))
                return;

            string oldName = SelectedKey.Modifier.Name;

            // Retrieve the new name from the user
            var dialog = new ChangeModifierNameDialog(oldName, ModifiersControl.Modifiers);
            if (dialog.ShowDialog() == true)
            {
                string newName = dialog.ModifierName;

                // 1. Inform the modifier control that a name is to be changed.
                ModifiersControl.ChangeModifierName(oldName, newName);
                // 2. Update each key registered as that modifier, so that it's updated
                Layer.RefreshView(SelectedKey.Modifier, SelectedKey.IsModifierSelected);
                // 3. Go through each layer that references the old name and update it
                foreach (var layer in _Layers)
                {
                    int index = layer.ModifierCombination.IndexOf(oldName);
                    if (index != -1)
                        layer.ModifierCombination[index] = newName;
                }
                UpdateButtonStates();
                KeyClicked?.Invoke(this, new KeyClickedEventArgs()
                {
                    Command = SelectedKey.Command,
                    KeyboardName = UniqueName,
                    Modifier = SelectedKey.Modifier,
                    Scancode = SelectedKey.Scancode,
                    KeyControl = SelectedKey,
                });
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
                UpdateButtonStates();
                KeyClicked?.Invoke(this, new KeyClickedEventArgs()
                {
                    Command = SelectedKey.Command,
                    KeyboardName = UniqueName,
                    Modifier = null,
                    Scancode = SelectedKey.Scancode,
                    KeyControl = SelectedKey,
                });
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

                _Layers.Add(_activeLayer);
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


        /// <summary>
        /// Called when the user wants to change the physical and logical layouts that determine the
        /// appearance of the control.
        /// </summary>
        private void ButtonChangeLayout_Click(object sender, RoutedEventArgs e)
        {
            var layoutDialog = new ChangePhysicalLayoutDialog();
            if (layoutDialog.ShowDialog() == true)
            {
                // Retrieve information from the dialog
                PhysicalLayoutStandard physicalStandard = layoutDialog.PhysicalLayout;
                string logicalLayout = layoutDialog.LogicalLayout;

                // Prepare the layer control with the new information, then tell it to rerender.
                Layer.Labels = LogicalLayoutFactory.GetLogicalLayout(logicalLayout);
                Layer.SetLayoutToRender(new DomainFacade().GetPhysicalLayout(physicalStandard));
                Layer.RefreshView(_activeLayer.Commands, ModifiersControl.Modifiers);
                // Then save that information in this keyboard, because it'll be persisted.
                PhysicalLayout = physicalStandard;
                LogicalLayout = logicalLayout;
            }
        }

        /// <summary>
        /// Called when the user wants to delete this keyboard. Sends an event to this control's owner,
        /// which is responsible for removing it.
        /// </summary>
        private void ButtonDeleteKeyboard_Click(object sender, RoutedEventArgs e)
        {
            // Send the event that signifies that this object is requesting deletion
            KeyboardDeletionRequest?.Invoke(this, new EventArgs());
        }

        private void ButtonKeyboardAliasEdit_Click(object sender, RoutedEventArgs e)
        {
            if (!_EditingAlias)
            {
                // Start editing the alias
                _EditingAlias = true;
                LabelKeyboardAlias.Width = 0;
                InputKeyboardAlias.Width = 200;
                InputKeyboardAlias.Text = LabelKeyboardAlias.Text;
                ButtonKeyboardAliasEdit.Content = Properties.Strings.KeyboardAliasDoneEditing;
            }
            else
            {
                // Stop editing the alias
                _EditingAlias = false;
                LabelKeyboardAlias.Width = double.NaN;
                InputKeyboardAlias.Width = 0;
                Alias = InputKeyboardAlias.Text;
                LabelKeyboardAlias.Text = InputKeyboardAlias.Text;
                ButtonKeyboardAliasEdit.Content = Properties.Strings.KeyboardAliasEdit;
            }
        } // TODO: A cancel button for editing the alias
    }
}

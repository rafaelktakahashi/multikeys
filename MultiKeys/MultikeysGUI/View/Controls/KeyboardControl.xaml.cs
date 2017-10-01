﻿using System;
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
        public string KeyboardName { get; set; }
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
            Layer.RenderLayout(physLayout);
            Layer.RefreshView(kb.Layers[0].Commands);
        }

        /// <summary>
        /// This method extracts a Keyboard object from this control.
        /// </summary>
        /// <returns></returns>
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

        #endregion


        /// <summary>
        /// This event occurs whenever a key is clicked.
        /// The parent control is responsible for displaying detailed information about the selected key.
        /// </summary>
        public event KeyClickedDelegate KeyClicked;

        // Event handlers
        public void LayerKeyClicked(object sender, EventArgs e)
        {
            // Bubble up the event
            var keyControl = sender as KeyControl;
            KeyClicked?.Invoke(this, new KeyClickedEventArgs
            {
                Command = keyControl.Command,
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
                    Layer.RefreshView(layer.Commands);
                    // done
                    return;
                }
            }

            // No layer was found
            // render no command
            Layer.RefreshView(EmptyLayerCommands);

        }

        private void ButtonAssignCommand_Click(object sender, RoutedEventArgs e)
        {
            // 
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
    }
}

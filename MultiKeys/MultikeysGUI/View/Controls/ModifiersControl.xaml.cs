using MultikeysGUI.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MultikeysGUI.View.Controls
{
    /// <summary>
    /// Interaction logic for ModifiersControl.xaml
    /// This class represents a set of modifiers, and is capable of
    /// keeping track of their states.
    /// </summary>
    public partial class ModifiersControl : UserControl
    {
        public ModifiersControl()
        {
            InitializeComponent();
            Modifiers = new List<Modifier>();
            SelectedModifiers = new List<string>();
            ModifierButtonList = new List<ToggleButton>();

            lblModifiers.Text = Properties.Strings.Modifiers;
        }

        /// <summary>
        /// Collection of all modifiers registered in this keyboard.
        /// </summary>
        public IEnumerable<Modifier> Modifiers { get; private set; }

        /// <summary>
        /// Collection containing only the modifiers registered in this keyboard
        /// that are set as active in this control.
        /// </summary>
        public IEnumerable<string> SelectedModifiers { get; private set; }

        /// <summary>
        /// List of references to the buttons that represent each modifier.
        /// </summary>
        private IList<ToggleButton> ModifierButtonList;

        /// <summary>
        /// Register a new modifier key; if a modifier with the specified name is already
        /// registered, then the scancode is added to it.
        /// </summary>
        /// <param name="sc"></param>
        /// <param name="name"></param>
        public void RegisterModifier(Scancode sc, string name)
        {
            // Try to retrieve a modifier with the same name
            var existingModifier =
                Modifiers
                .Where(it => it.Name == name)
                .DefaultIfEmpty(null)
                .Single();
            // If no modifier could be found, then this is a new one
            if (existingModifier != null)
            {
                existingModifier.Scancodes.Add(sc);
            }
            else
            {
                // otherwise, add a new modifier with the specified scancode
                (Modifiers as List<Modifier>).Add(
                    new Modifier { Name = name, Scancodes = { sc } }
                    );
                // and also add a new button for that modifier
                ToggleButton newToggleButton = this.Resources["ModifierButton"] as ToggleButton;
                newToggleButton.Name = name;
                // Subscribe to its toggled event
                newToggleButton.Checked += ModifierClicked;
                newToggleButton.Unchecked += ModifierClicked;
                // add to layout
                ModifierButtons.Children.Add(newToggleButton);
                // and keep a reference to it
                ModifierButtonList.Add(newToggleButton);
            }

            // Fire an event
            ModifierSelectionChanged?.Invoke(this, new EventArgs());
        }

        /// <summary>
        /// Unregisters a scancode as modifier, then removes the modifier if no scancode
        /// is associated with it.
        /// </summary>
        /// <param name="sc"></param>
        public void RemoveModifier(Scancode sc)
        {
            (Modifiers as List<Modifier>)
                .ForEach((it) =>
                {
                    it.Scancodes.Remove(sc);
                });
            (Modifiers as List<Modifier>)
                .RemoveAll(it => it.Scancodes.Count == 0);

            // Fire an event
            ModifierSelectionChanged?.Invoke(this, new EventArgs());
        }



        /// <summary>
        /// Event that fires whenever modifiers are registered or unregistered,
        /// and also whenever the combination of selected modifiers changes.
        /// </summary>
        public event EventHandler ModifierSelectionChanged;

        /// <summary>
        /// Handles each button in this control.
        /// </summary>
        public void ModifierClicked(object sender, RoutedEventArgs e)
        {
            ModifierSelectionChanged?.Invoke(sender, new EventArgs());
        }

        



    }
}

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
            ActiveModifiers = new List<string>();

            lblModifiers.Text = Properties.Strings.Modifiers;
        }

        /// <summary>
        /// Collection of all modifiers registered in this keyboard.
        /// </summary>
        public IEnumerable<Modifier> Modifiers { get; private set; }

        /// <summary>
        /// Register a new modifier key; if a modifier with the specified name is already
        /// registered, then the specified scancode is added to it.
        /// </summary>
        /// <param name="sc"></param>
        /// <param name="name"></param>
        public void RegisterModifier(Scancode sc, string name)
        {
            var existingModifier =
                Modifiers
                .Where(it => it.Name == name)
                .DefaultIfEmpty(null)
                .Single();
            if (existingModifier != null)
            {
                existingModifier.Scancodes.Add(sc);
            }
            else
            {
                (Modifiers as List<Modifier>).Add(
                    new Modifier { Name = name, Scancodes = { sc } }
                    );
            }

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

            ModifierSelectionChanged?.Invoke(this, new EventArgs());
        }

        /// <summary>
        /// Collection containing only the modifiers registered in this keyboard
        /// that are set as active in this control.
        /// </summary>
        public IEnumerable<string> ActiveModifiers { get; private set; }



        /// <summary>
        /// Event that fires whenever modifiers are registered or unregistered,
        /// and also whenever the combination of selected modifiers changes.
        /// </summary>
        public event EventHandler ModifierSelectionChanged;


    }
}

using MultikeysEditor.Model;
using System.Collections.Generic;
using System.Windows;

namespace MultikeysEditor.View.Dialogues
{
    /// <summary>
    /// Interaction logic for ModifierDialog.xaml
    /// </summary>
    public partial class ModifierDialog : Window
    {

        public ModifierDialog(ICollection<Modifier> modifiers)
        {
            InitializeComponent();

            foreach (var mod in modifiers)
            {
                ComboExistingModifiers.Items.Add(mod.Name);
            }

            ModifierName = null;
        }

        /// <summary>
        /// Contains the name of the new modifier when this dialog closes correctly.
        /// If this dialog did not close correctly (that is, DialogResult is false), this property remains null.
        /// </summary>
        public string ModifierName { get; set; }


        private void RadioChanged(object sender, RoutedEventArgs e)
        {
            if (InputNewModifier == null) return;
            if (ComboExistingModifiers == null) return;

            if (RadioNewModifier.IsChecked ?? false)
            {
                InputNewModifier.IsEnabled = true;
                ComboExistingModifiers.IsEnabled = false;
            }
            else
            {
                InputNewModifier.IsEnabled = false;
                ComboExistingModifiers.IsEnabled = true;
            }
        }

        private void Confirm(object sender, RoutedEventArgs e)
        {
            if (RadioNewModifier.IsChecked ?? false)
            {
                if (string.IsNullOrEmpty(InputNewModifier.Text))
                {
                    return;
                }
                else
                {
                    ModifierName = InputNewModifier.Text;
                    DialogResult = true;
                }
            }
            else
            {
                if (ComboExistingModifiers.Items.Count == 0)
                    return;
                if (ComboExistingModifiers.SelectedValue == null)
                    return;
                ModifierName = ComboExistingModifiers.SelectedValue as string;
                DialogResult = true;
            }
        }
    }
}

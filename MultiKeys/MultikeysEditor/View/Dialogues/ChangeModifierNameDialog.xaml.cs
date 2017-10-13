using MultikeysEditor.Model;
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
using System.Windows.Shapes;

namespace MultikeysEditor.View.Dialogues
{
    /// <summary>
    /// Interaction logic for ChangeModifierNameDialog.xaml
    /// </summary>
    public partial class ChangeModifierNameDialog : Window
    {
        public ChangeModifierNameDialog(string currentName, IEnumerable<Modifier> existingModifiers)
        {
            InitializeComponent();

            _existingModifiers = existingModifiers;

            ModifierName = null;
            InputNewModifierName.Text = currentName;

            // force an update right away
            InputNewModifierName_TextChanged(null, null);
        }


        private IEnumerable<Modifier> _existingModifiers;

        private bool _modifierNameIsInvalid;


        /// <summary>
        /// This property will contain the new modifier name if this dialog exited correctly.
        /// If this dialog existed incorrectly (DialogResult is false), then this will remain null.
        /// </summary>
        public string ModifierName { get; private set; }

        private void ButtonConfirm_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrWhiteSpace(InputNewModifierName.Text)
                || _modifierNameIsInvalid)
            {
                MessageBox.Show(
                    Properties.Strings.PromptChangeModifierNameInvalidText,
                    Properties.Strings.Warning,
                    MessageBoxButton.OK,
                    MessageBoxImage.Warning);
                return;
            }

            ModifierName = InputNewModifierName.Text;
            DialogResult = true;
        }

        private void InputNewModifierName_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (sender == null)
                return;

            if (_existingModifiers.Any(mod => mod.Name == (sender as TextBox).Text))
            {
                LabelNameAlreadyExists.Visibility = Visibility.Visible;
                _modifierNameIsInvalid = true;
            }
            else
            {
                LabelNameAlreadyExists.Visibility = Visibility.Hidden;
                _modifierNameIsInvalid = false;
            }
        }
    }
}

using MultikeysEditor.Domain.Layout;
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
    /// Interaction logic for ChangePhysicalLayoutDialog.xaml
    /// </summary>
    public partial class ChangePhysicalLayoutDialog : Window
    {
        public ChangePhysicalLayoutDialog()
        {
            InitializeComponent();

            ComboBoxStandards.Items.Add("ANSI");
            ComboBoxStandards.Items.Add("ISO");
            ComboBoxStandards.Items.Add("ABNT-2");
            ComboBoxStandards.Items.Add("JIS");
            ComboBoxStandards.Items.Add("Dubeolsik");
        }

        /// <summary>
        /// This property should only be accessed if this dialog returns true on ShowDialog().
        /// It will contain the physical layout chosen by the user.
        /// </summary>
        public PhysicalLayoutStandard Standard { get; private set; }

        /// <summary>
        /// This property should only be accessed if this dialog returns true on ShowDialog().
        /// It will be true if the user chose to use a big return key in the layout.
        /// </summary>
        public bool UseBigReturn { get; private set; }

        private void ButtonConfirm_Click(object sender, RoutedEventArgs e)
        {
            if (ComboBoxStandards.SelectedValue != null)
            {
                string value = ComboBoxStandards.SelectedValue as string;
                switch (value)
                {
                    case "ANSI":
                        Standard = PhysicalLayoutStandard.ANSI;
                        break;
                    case "ISO":
                        Standard = PhysicalLayoutStandard.ISO;
                        break;
                    case "ABNT-2":
                        Standard = PhysicalLayoutStandard.ABNT_2;
                        break;
                    case "JIS":
                        Standard = PhysicalLayoutStandard.JIS;
                        break;
                    case "Dubeolsik":
                        Standard = PhysicalLayoutStandard.DUBEOLSIK;
                        break;
                    default:
                        Standard = PhysicalLayoutStandard.ISO;
                        break;
                }
                UseBigReturn = CheckBoxBigEnter.IsChecked ?? false;
                DialogResult = true;    // will close this window
            }
            
        }

    }
}

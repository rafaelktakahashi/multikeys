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

            ComboBoxPhysicalStandards.Items.Add("ANSI");
            ComboBoxPhysicalStandards.Items.Add("ISO");
            ComboBoxPhysicalStandards.Items.Add("ABNT-2");
            ComboBoxPhysicalStandards.Items.Add("JIS");
            ComboBoxPhysicalStandards.Items.Add("Dubeolsik");

            ComboBoxLogicalLayouts.Items.Add("ANSI US - English");
            ComboBoxLogicalLayouts.Items.Add("ISO UK - English");
            ComboBoxLogicalLayouts.Items.Add("ISO DE - Deutsch");
            ComboBoxLogicalLayouts.Items.Add("ISO FR - Français");
            ComboBoxLogicalLayouts.Items.Add("ISO ES - Español");
            ComboBoxLogicalLayouts.Items.Add("ABNT-2 - Português Brasileiro");
            ComboBoxLogicalLayouts.Items.Add("JIS - 日本語");
            ComboBoxLogicalLayouts.Items.Add("Dubeolsik - 한국어");
        }

        /// <summary>
        /// This property should only be accessed if this dialog returns true on ShowDialog().
        /// It will contain the physical layout chosen by the user.
        /// </summary>
        public PhysicalLayoutStandard PhysicalLayout { get; private set; }

        /// <summary>
        /// This property should only be accessed if this dialog returns true on ShowDialog().
        /// It will contain the physical layout chosen by the user.
        /// </summary>
        public string LogicalLayout { get; private set; }

        /// <summary>
        /// This property should only be accessed if this dialog returns true on ShowDialog().
        /// It will be true if the user chose to use a big return key in the layout.
        /// </summary>
        public bool UseBigReturn { get; private set; }

        private void ButtonConfirm_Click(object sender, RoutedEventArgs e)
        {
            if (ComboBoxPhysicalStandards.SelectedItem != null
                && ComboBoxLogicalLayouts.SelectedItem != null)
            {
                string value = ComboBoxPhysicalStandards.SelectedValue as string;
                switch (value)
                {
                    case "ANSI":
                        PhysicalLayout = PhysicalLayoutStandard.ANSI;
                        break;
                    case "ISO":
                        PhysicalLayout = PhysicalLayoutStandard.ISO;
                        break;
                    case "ABNT-2":
                        PhysicalLayout = PhysicalLayoutStandard.ABNT_2;
                        break;
                    case "JIS":
                        PhysicalLayout = PhysicalLayoutStandard.JIS;
                        break;
                    case "Dubeolsik":
                        PhysicalLayout = PhysicalLayoutStandard.DUBEOLSIK;
                        break;
                    default:
                        PhysicalLayout = PhysicalLayoutStandard.ISO;
                        break;
                }
                UseBigReturn = CheckBoxBigEnter.IsChecked ?? false;

                value = ComboBoxLogicalLayouts.SelectedValue as string;
                switch (value)
                {
                    case "ANSI US - English":
                        LogicalLayout = "US";
                        break;
                    case "ISO UK - English":
                        LogicalLayout = "UK";
                        break;
                    case "ISO DE - Deutsch":
                        LogicalLayout = "DE";
                        break;
                    case "ISO FR - Français":
                        LogicalLayout = "FR";
                        break;
                    case "ISO ES - Español":
                        LogicalLayout = "ES";
                        break;
                    case "ABNT-2 - Português Brasileiro":
                        LogicalLayout = "ABNT_2";
                        break;
                    case "JIS - 日本語":
                        LogicalLayout = "JIS";
                        break;
                    case "Dubeolsik - 한국어":
                        LogicalLayout = "Dubeolsik";
                        break;

                    default: LogicalLayout = "US"; break;
                }

                DialogResult = true;    // will close this window
            }
            
        }

    }
}

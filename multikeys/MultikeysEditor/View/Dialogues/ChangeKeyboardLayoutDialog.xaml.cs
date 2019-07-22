using MultikeysEditor.Domain.Layout;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Xml.Linq;

namespace MultikeysEditor.View.Dialogues
{
    /// <summary>
    /// Interaction logic for ChangePhysicalLayoutDialog.xaml
    /// </summary>
    public partial class ChangePhysicalLayoutDialog : Window
    {

        private class LogicalLayoutFile
        {
            /// <summary>
            /// User-friendly name of this layout
            /// </summary>
            public string Name { get; set; }
            /// <summary>
            /// Name of the file that contains this layout, without path or extension
            /// </summary>
            public string File { get; set; }
            public override string ToString()
            {
                return Name;
            }
        }

        public ChangePhysicalLayoutDialog(PhysicalLayoutStandard startingPhysicalLayout, string startingLogicalLayout)
        {
            InitializeComponent();

            ComboBoxPhysicalStandards.Items.Add("ANSI");
            ComboBoxPhysicalStandards.Items.Add("ISO");
            ComboBoxPhysicalStandards.Items.Add("ABNT-2");
            ComboBoxPhysicalStandards.Items.Add("JIS");
            ComboBoxPhysicalStandards.Items.Add("Dubeolsik");


            // Read index of logical layouts (which indexes names and the files where they're located)
            string pathToIndex =
                System.IO.Path
                .GetDirectoryName(
                    System.Reflection.Assembly
                    .GetEntryAssembly()
                    .Location
                    )
                    + @"\Resources\Layouts\index.xml";

            XDocument xml = XDocument.Load(pathToIndex);
            var query = from layout in xml.Root.Elements("layout")
                        select new LogicalLayoutFile
                        { Name = layout.Element("name").Value, File = layout.Element("file").Value };
            LogicalLayoutFiles = query.ToList();

            // Setup the combobox
            ComboBoxLogicalLayouts.DataContext = this;
            ComboBoxLogicalLayouts.ItemsSource = LogicalLayoutFiles;
            ComboBoxLogicalLayouts.SelectedValuePath = "File";

            // Try to set the selected item
            switch (startingPhysicalLayout)
            {
                case PhysicalLayoutStandard.ABNT_2:
                case PhysicalLayoutStandard.ABNT_2_BIGRETURN:
                    ComboBoxPhysicalStandards.SelectedItem = "ABNT-2";
                    break;
                case PhysicalLayoutStandard.ANSI:
                case PhysicalLayoutStandard.ANSI_BIGRETURN:
                    ComboBoxPhysicalStandards.SelectedItem = "ANSI";
                    break;
                case PhysicalLayoutStandard.DUBEOLSIK:
                case PhysicalLayoutStandard.DUBEOLSIK_BIGRETURN:
                    ComboBoxPhysicalStandards.SelectedItem = "Dubeolsik";
                    break;
                case PhysicalLayoutStandard.ISO:
                case PhysicalLayoutStandard.ISO_BIGRETURN:
                    ComboBoxPhysicalStandards.SelectedItem = "ISO";
                    break;
                case PhysicalLayoutStandard.JIS:
                    ComboBoxPhysicalStandards.SelectedItem = "JIS";
                    break;
                default: break; // Leave no selected item
            }

            var layoutFileIndex = LogicalLayoutFiles.FindIndex(it => it.File == startingLogicalLayout);
            if (layoutFileIndex >= 0)
            {
                ComboBoxLogicalLayouts.SelectedIndex = layoutFileIndex;
            }
        }


        private List<LogicalLayoutFile> LogicalLayoutFiles { get; set; }


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

        private void ButtonConfirm_Click(object sender, RoutedEventArgs e)
        {
            if (ComboBoxPhysicalStandards.SelectedItem != null
                && ComboBoxLogicalLayouts.SelectedItem != null)
            {
                string value = ComboBoxPhysicalStandards.SelectedValue as string;
                if (CheckBoxBigEnter.IsChecked ?? false) switch (value)
                    {
                        case "ANSI":
                            PhysicalLayout = PhysicalLayoutStandard.ANSI_BIGRETURN;
                            break;
                        case "ISO":
                            PhysicalLayout = PhysicalLayoutStandard.ISO_BIGRETURN;
                            break;
                        case "ABNT-2":
                            PhysicalLayout = PhysicalLayoutStandard.ABNT_2_BIGRETURN;
                            break;
                        case "JIS":
                            PhysicalLayout = PhysicalLayoutStandard.JIS;
                            break;
                        case "Dubeolsik":
                            PhysicalLayout = PhysicalLayoutStandard.DUBEOLSIK_BIGRETURN;
                            break;
                        default:
                            PhysicalLayout = PhysicalLayoutStandard.ISO_BIGRETURN;
                            break;
                    }
                else switch (value)
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

                LogicalLayout = ComboBoxLogicalLayouts.SelectedValue as string;

                DialogResult = true;    // will close this window
            }
            
        }

    }
}

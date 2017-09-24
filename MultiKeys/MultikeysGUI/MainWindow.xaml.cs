using MultikeysGUI.Domain.Layout;
using MultikeysGUI.Model;
using MultikeysGUI.View.Controls;
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


namespace MultikeysGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            var command = new DeadKeyCommand
            {
                Codepoints = new List<uint> { 0x1f604, 0x40 },
                Replacements = new Dictionary<IList<uint>, IList<uint>>
                {
                    {
                        new List<uint> { 0x30, 0x31 },
                        new List<uint> { 0x1f630, 0x1f650 }
                    }
                }
            };
            var command2 = new UnicodeCommand(true, char.ConvertFromUtf32(0x1f604));
            Dictionary<Scancode, IKeystrokeCommand> layout = new Dictionary<Scancode, IKeystrokeCommand>
            {
                { Scancode.FromString("04"), command },
                { Scancode.FromString("e0:52"), command2 }
            };


            ExampleLayout.LoadLayout(PhysicalLayoutFactory.FromStandard(PhysicalLayoutStandard.ISO));
            ExampleLayout.RefreshView(layout);
        }


        public void HandleKeyClicked(object sender, EventArgs e)
        {
            SummaryPanel.UpdateCommand( (sender as KeyControl).Command );
        }
        
    }
}

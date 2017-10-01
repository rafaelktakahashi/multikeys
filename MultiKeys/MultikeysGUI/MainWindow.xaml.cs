using MultikeysGUI.Domain;
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


            // Initializing the facade
            IDomainFacade applicationFacade = new DomainFacade();

            var layout = applicationFacade.LoadLayout(@"C:\Users\Rafael\git\Multikeys\MultiKeys\XML\Multikeys.xml");

            foreach (var keyboard in layout.Keyboards)
            {
                var kbControl = new KeyboardControl(keyboard, applicationFacade.GetPhysicalLayout(PhysicalLayoutStandard.ANSI, true))
                {
                    Width = 2000,
                    Height = 480,
                    Margin = new Thickness(5),
                };
                kbControl.KeyClicked += HandleKeyClicked;
                KeyboardStack.Children.Add(kbControl);
            }
            
            

            
        }

        

        /// <summary>
        /// This event handler is notified of any key that is clicked on any keyboard on screen.
        /// </summary>
        public void HandleKeyClicked(object sender, KeyClickedEventArgs e)
        {
            // Use the provided information to update the information panel.
            SummaryPanel.UpdateCommand(e.Command);
        }
        
    }
}

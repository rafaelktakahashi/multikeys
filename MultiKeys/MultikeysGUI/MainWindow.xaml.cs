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

            var kbControl = new KeyboardControl(layout.Keyboards[0], applicationFacade.GetPhysicalLayout(PhysicalLayoutStandard.ISO))
            {
                Width = 1300,
                Height = 400,
                Margin = new Thickness(5),
            };

            Root.Children.Add(kbControl);

            
        }

        

        public void HandleKeyClicked(object sender, EventArgs e)
        {
            ;
        }
        
    }
}

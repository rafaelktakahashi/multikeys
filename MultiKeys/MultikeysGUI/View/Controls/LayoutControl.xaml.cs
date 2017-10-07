using MultikeysGUI.Domain;
using MultikeysGUI.Domain.Layout;
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

namespace MultikeysGUI.View.Controls
{
    /// <summary>
    /// Interaction logic for LayoutControl.xaml
    /// </summary>
    public partial class LayoutControl : UserControl
    {
        public LayoutControl()
        {
            InitializeComponent();
        }

        public void LoadLayout(MultikeysLayout layout)
        {
            IDomainFacade applicationFacade = new DomainFacade();

            foreach (var keyboard in layout.Keyboards)
            {                                                                       // TODO: Solve the physical layout
                var kbControl = new KeyboardControl(keyboard, applicationFacade.GetPhysicalLayout(PhysicalLayoutStandard.ISO, useBigReturn: false))
                {
                    Width = double.NaN,     // NaN means Auto
                    Height = double.NaN,
                    Margin = new Thickness(5, 5, 5, 20),
                };
                kbControl.KeyClicked += HandleKeyClicked;
                KeyboardStack.Children.Add(kbControl);
            }

        }


        /// <summary>
        /// Builds a MultikeysLayout instance from this control.
        /// </summary>
        public MultikeysLayout GetLayout()
        {
            var layout = new MultikeysLayout();
            // TODO: Add each keyboard
            foreach (var control in KeyboardStack.Children)
            {
                if (control is KeyboardControl)
                {
                    // get each keyboard from the keyboard controls
                    var keyboard = (control as KeyboardControl).GetKeyboard();
                    layout.Keyboards.Add(keyboard);
                }
            }
            return layout;
        }


        /// <summary>
        /// This event handler is notified of any key that is clicked on any keyboard on screen.
        /// </summary>
        public void HandleKeyClicked(object sender, KeyClickedEventArgs e)
        {
            if (e.Modifier != null)
                SummaryPanel.UpdateCommand(e.Modifier);
            else
                SummaryPanel.UpdateCommand(e.Command);
        }



    }
}

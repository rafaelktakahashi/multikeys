using MultikeysEditor.Domain;
using MultikeysEditor.Domain.Layout;
using MultikeysEditor.Model;
using System;
using System.Windows;
using System.Windows.Controls;

namespace MultikeysEditor.View.Controls
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
                var kbControl = new KeyboardControl(keyboard, PhysicalLayoutStandard.ISO)
                {
                    Width = double.NaN,     // NaN means Auto
                    Height = double.NaN,
                    Margin = new Thickness(5, 5, 5, 20),
                };
                kbControl.KeyClicked += HandleKeyClicked;
                kbControl.KeyboardDeletionRequest += KeyboardDeletion;
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



        /// <summary>
        /// Adds the keyboards in the specified layout into this one, without overriding any existing keyboards.
        /// </summary>
        /// <param name="layout"></param>
        public void AddKeyboards(MultikeysLayout layout)
        {
            IDomainFacade applicationFacade = new DomainFacade();

            foreach (var keyboard in layout.Keyboards)
            {                                                                       // TODO: Solve the physical layout
                var kbControl = new KeyboardControl(keyboard, PhysicalLayoutStandard.ISO)
                {
                    Width = double.NaN,     // NaN means Auto
                    Height = double.NaN,
                    Margin = new Thickness(5, 5, 5, 20),
                };
                kbControl.KeyClicked += HandleKeyClicked;
                kbControl.KeyboardDeletionRequest += KeyboardDeletion;
                KeyboardStack.Children.Add(kbControl);
            }

            // Scroll to last keyboard, which was just added.
            KeyboardScrollViewer.ScrollToEnd();
        }


        /// <summary>
        /// Adds a new keyboard control, properly initialized. It is placed at the end of the list.
        /// </summary>
        private void ButtonAddNewKeyboard_Click(object sender, RoutedEventArgs e)
        {                                                                               // solve the thing with the physical layouts
            var kbControl = new KeyboardControl(new Model.Keyboard(), PhysicalLayoutStandard.ISO)
            {                                   // the keyboard contructor initializes all of the object's contents.
                Width = double.NaN,     // NaN means Auto
                Height = double.NaN,
                Margin = new Thickness(5, 5, 5, 20),
            };
            kbControl.KeyClicked += HandleKeyClicked;
            kbControl.KeyboardDeletionRequest += KeyboardDeletion;
            KeyboardStack.Children.Add(kbControl);

            // Automatically scroll the user to the bottom of the layout, where the new keyboard was placed
            KeyboardScrollViewer.ScrollToEnd();
        }



        /// <summary>
        /// This handler is called when a keyboard requests to be deleted.
        /// Deletes the keyboard that requested to be deleted.
        /// Assumes that the sender is the keyboard that requested to be deleted.
        /// </summary>
        private void KeyboardDeletion(object sender, EventArgs e)
        {
            foreach (var control in KeyboardStack.Children)
            {
                if (control is KeyboardControl && control.Equals(sender))
                {
                    KeyboardStack.Children.Remove(sender as UIElement);
                    // The KeyboardControls in KeyboardStack represent the keyboards in this control.
                    return;
                }
            }
        }
    }
}

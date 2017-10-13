using MultikeysGUI.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Unicode;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace MultikeysGUI.View.Dialogues
{
    /// <summary>
    /// Interaction logic for KeystrokeCommandDialog.xaml
    /// </summary>
    public partial class KeystrokeCommandDialog : Window
    {
        public KeystrokeCommandDialog()
        {
            InitializeComponent();

            // Set the data context for this window
            RootGrid.DataContext = this;

            // Initialize the command to null
            Command = null;

            /* Unicode */
            // Setup the timer
            _unicodeDelay = new System.Timers.Timer
            {
                Interval = 1000,        // One second
                AutoReset = false,      // Call its event only once.
            };
            _unicodeDelay.Elapsed += (object sender, System.Timers.ElapsedEventArgs e) => UnicodeUpdateList();

            // setup the list of characters
            UnicodeCharactersSource = new ObservableCollection<UnicodeCharacterItem>();
            UnicodeCharactersList.ItemsSource = UnicodeCharactersSource;
            // update the list right away
            UnicodeUpdateList();
        }


        /// <summary>
        /// Contains the newly created command when this window closes correctly.
        /// If this window closes incorrectly (DialogResult is false), then this property remains null.
        /// </summary>
        public IKeystrokeCommand Command { get; private set; }
        


        /// <summary>
        /// Call this method if this dialog is meant to edit an existing command.
        /// Note that the Command property in this dialog will remain uninitialized.
        /// </summary>
        public void UseExistingCommand(IKeystrokeCommand command)
        {
            // Update the text
            this.Title = Properties.Strings.WindowTitleEditCommandDialog;

            // Call the correct private method depending on the command's type
            if (command is DeadKeyCommand)
            {
                CommandTabControl.SelectedIndex = 2;
            }
            else if (command is UnicodeCommand)
            {
                UseExistingUnicodeCommand(command as UnicodeCommand);
                CommandTabControl.SelectedIndex = 0;
            }
            else return;
        }


        #region Unicode
        
        public class UnicodeCharacterItem : INotifyPropertyChanged
        {
            private string _character;
            public string Character
            {
                get { return _character; }
                set { _character = value; NotifyPropertyChanged("Character"); }
            }
            private string _codepoint;
            public string Codepoint
            {
                get { return _codepoint; }
                set { _codepoint = value; NotifyPropertyChanged("Codepoint"); }
            }
            public event PropertyChangedEventHandler PropertyChanged;
            private void NotifyPropertyChanged(string propertyName)
            { PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName)); }
        }

        /// <summary>
        /// Changing the text in the input textbox should cause the unicode characters list to update,
        /// but with a delay. This object is used to implement that delay, in order to reduce lag with very long texts.
        /// </summary>
        private System.Timers.Timer _unicodeDelay;

        /// <summary>
        /// List of codepoints written on the input textbox.
        /// </summary>
        public ObservableCollection<UnicodeCharacterItem> UnicodeCharactersSource { get; set; }

        /// <summary>
        /// Call this method internally to set an existing command to be edited.
        /// </summary>
        /// <param name="command"></param>
        private void UseExistingUnicodeCommand(UnicodeCommand command)
        {
            UnicodeTextInput.Text = command.ContentAsText;
            UnicodeUpdateList();
        }

        /// <summary>
        /// Called when the text in the input textbox changes.
        /// Updates the list of characters, but with a delay.
        /// </summary>
        private void UnicodeTextInput_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_unicodeDelay == null) return;
            // Start the timer, or reset it if it was already running
            _unicodeDelay.Stop();
            _unicodeDelay.Start();
        }

        /// <summary>
        /// Called when the timer setup by UnicodeTextInput_TextChanged times out.
        /// Updates the list of characters.
        /// </summary>
        private void UnicodeUpdateList()
        {
            // This is called from a timer. Thus, a dispatcher is needed to access the window's controls.
            Dispatcher.Invoke(() =>
            {
                _unicodeDelay.Stop();
                UnicodeCharactersSource.Clear();

                for (int i = 0; i < UnicodeTextInput.Text.Length; i++)
                {
                    int codepoint = char.ConvertToUtf32(UnicodeTextInput.Text, i);

                    if (codepoint > 0xffff)
                        i++;

                    UnicodeCharactersSource.Add(
                        new UnicodeCharacterItem() { Character=char.ConvertFromUtf32(codepoint), Codepoint="U+"+codepoint.ToString("X")}
                        );
                }
            });
        }

        private void UnicodeCharactersList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (e.AddedItems.Count > 0)
            {
                var currentSelection = e.AddedItems[0] as UnicodeCharacterItem;
                // Display the character
                UnicodeSelectedCharacterDisplay.Content = currentSelection.Character;
                // Display details
                int codepoint = char.ConvertToUtf32(currentSelection.Character, 0);
                var info = UnicodeInfo.GetCharInfo(char.ConvertToUtf32(currentSelection.Character, 0));
                PromptUnicodeSelectedCharacterName.Content = info.Name;
                PromptUnicodeSelectedCharacterCodepoint.Content = currentSelection.Codepoint;
                PromptUnicodeSelectedCharacterBlock.Content = info.Block;
                PromptUnicodeSelectedCharacterCategory.Content = info.Category;
            }
        }

        /// <summary>
        /// Creates a new command.
        /// </summary>
        private void PromptUnicodeConfirm_Click(object sender, RoutedEventArgs e)
        {
            Command =
                new UnicodeCommand(PromptUnicodeTriggerOnRepeat.IsChecked ?? false, UnicodeTextInput.Text);
            DialogResult = true;    // This closes the window
        }

        #endregion


    }
}

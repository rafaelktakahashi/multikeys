using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
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

            _unicodeDelay = new System.Timers.Timer
            {
                Interval = 1000,        // One second
                AutoReset = false,      // Call its event only once.
            };
            _unicodeDelay.Elapsed += UnicodeTextInput_TextChanged_TimeUp;

            UnicodeCharactersSource = new ObservableCollection<UnicodeCharacterItem>();
            UnicodeCharactersList.ItemsSource = UnicodeCharactersSource;
            UnicodeCharactersList.DataContext = this;
            UnicodeTextInput_TextChanged_TimeUp(null, null);
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
        private void UnicodeTextInput_TextChanged_TimeUp(object sender, System.Timers.ElapsedEventArgs e)
        {
            // This is called from a timer. Thus, a dispatcher is needed to access the window's controls.
            Dispatcher.Invoke(() =>
            {
                _unicodeDelay.Stop();
                UnicodeCharactersSource.Clear();

                for (int i = 0; i < UnicodeTextInput.Text.Length; i++)
                {
                    int codepoint = char.ConvertToUtf32(UnicodeTextInput.Text, i);

                    if (codepoint > 0x1ffff)
                        i++;

                    UnicodeCharactersSource.Add(
                        new UnicodeCharacterItem() { Character=char.ConvertFromUtf32(codepoint), Codepoint="U+"+codepoint.ToString("X")}
                        );
                }
            });
        }

        /// <summary>
        /// Called when a character is selected in the list.
        /// Shows details about the selected character.
        /// </summary>
        private void ListBoxItem_Selected(object sender, RoutedEventArgs e)
        {

        }


        #endregion

        
    }
}

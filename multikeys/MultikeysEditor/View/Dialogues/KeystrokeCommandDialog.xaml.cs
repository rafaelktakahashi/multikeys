using MultikeysEditor.Model;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Unicode;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms;
using static MultikeysEditor.Model.DeadKeyCommand;

namespace MultikeysEditor.View.Dialogues
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
            // This timer doesn't run all the time; it's started when typing in the unicode
            // textbox, and when it finishes the list of characters is updated.
            _unicodeDelay = new System.Timers.Timer
            {
                Interval = 800,         // Interval in ms
                AutoReset = false,      // Call its event only once.
            };
            _unicodeDelay.Elapsed += (object sender, System.Timers.ElapsedEventArgs e) => UnicodeUpdateList();

            // setup the list of characters
            UnicodeCharactersSource = new ObservableCollection<UnicodeCharacterItem>();
            UnicodeCharactersList.ItemsSource = UnicodeCharactersSource;
            // update the list right away
            UnicodeUpdateList();

            // setup the list of replacements
            DeadKeyReplacementsSource = new ObservableCollection<DeadKeyReplacementItem>();
            DeadKeyReplacementsTable.ItemsSource = DeadKeyReplacementsSource;
            // Fix cell style
            DeadKeyReplacementsTable.CellStyle = new Style(typeof(System.Windows.Controls.DataGridCell))
            {
                Setters = { new Setter(TextBlock.TextAlignmentProperty, TextAlignment.Center) }
            };
        }


        /// <summary>
        /// Contains the newly created command when this window closes correctly.
        /// This property is only set at the moment the dialog is confirmed; that means that
        /// if this window closes incorrectly (DialogResult is false), then this property remains null.
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
                UseExistingDeadKeyCommand(command as DeadKeyCommand);
                CommandTabControl.SelectedIndex = 2;
            }
            else if (command is UnicodeCommand)
            {
                UseExistingUnicodeCommand(command as UnicodeCommand);
                CommandTabControl.SelectedIndex = 0;
            }
            else if (command is ExecutableCommand)
            {
                UseExistingExecutableCommand(command as ExecutableCommand);
                CommandTabControl.SelectedIndex = 1;
            } else if (command is MacroCommand)
            {
                // TODO: Call UseExistingMacroCommand(...) when it exists
                CommandTabControl.SelectedIndex = 3;
            }
        }

        /// <summary>
        /// Event listener for the main confirm button.
        /// </summary>
        private void PromptConfirm_Click(object sender, RoutedEventArgs e)
        {
            // Call the correct method depending on which tab is currently active.
            switch (CommandTabControl.SelectedIndex)
            {
                case 0:
                    PromptUnicodeConfirm_Click(sender, e);
                    break;
                case 1:
                    PromptExecutableConfirm_Click(sender, e);
                    break;
                case 2:
                    PromptDeadKeyConfirm_Click(sender, e);
                    break;
                default:
                    // Missing cases 2 and 3
                    return;
            }
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


        #region ExecutableCommand

        // The command and the arguments are stored in the textboxes and nowhere else.

        private void UseExistingExecutableCommand(ExecutableCommand command)
        {
            ExecutableCommandInput.Text = command.Command;
            ExecutableArgumentsInput.Text = command.Arguments;
        }

        private void ChooseExecutableFileButton_Click(object sender, RoutedEventArgs e)
        {
            var openFileDialog = new OpenFileDialog()
            {
                CheckFileExists = true,
                CheckPathExists = true,
                Filter = Properties.Strings.ExecutableFileFilter,
                DereferenceLinks = true,
                Multiselect = false,
            };
            var result = openFileDialog.ShowDialog();
            if (result == System.Windows.Forms.DialogResult.OK)
            {
                ExecutableCommandInput.Text = openFileDialog.FileName;
            }
        }

        private void TestExecutableCommandButton_Click(object sender, RoutedEventArgs e)
        {
            string errorMessage = null; // we'll use null to mean there's no error (so, success)
            var startInfo = new System.Diagnostics.ProcessStartInfo
            {
                FileName = ExecutableCommandInput.Text,
                Arguments = ExecutableArgumentsInput.Text,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false,
                CreateNoWindow = true
            };
            try
            {
                System.Diagnostics.Process.Start(startInfo);
            } catch (Exception exception)
            {
                errorMessage = exception.Message;
            }

            if (errorMessage != null)
            {
                // Error
                System.Windows.MessageBox.Show(this, errorMessage, Properties.Strings.PromptExecutableTestCommandFailed, MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }

        private void PromptExecutableConfirm_Click(object sender, RoutedEventArgs e)
        {
            Command =
                new ExecutableCommand()
                {
                    Command = ExecutableCommandInput.Text,
                    Arguments = ExecutableArgumentsInput.Text
                };
            DialogResult = true;
        }

        #endregion

        #region DeadKey

        public class DeadKeyReplacementItem : INotifyPropertyChanged
        {
            private string _from;
            private string _to;

            public string From
            {
                get { return _from; }
                set { _from = value; NotifyPropertyChanged("From"); }
            }
            public string To
            {
                get { return _to; }
                set { _to = value; NotifyPropertyChanged("To"); }
            }

            public event PropertyChangedEventHandler PropertyChanged;
            private void NotifyPropertyChanged(string propertyName)
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        public ObservableCollection<DeadKeyReplacementItem> DeadKeyReplacementsSource { get; set; }

        private void UseExistingDeadKeyCommand(DeadKeyCommand command)
        {
            // The list of replacements is expected to be initialized.
            DeadKeyIndependentTextInput.Text = command.ContentAsText;
            foreach (var replacementPair in command.ReplacementsAsList)
            {
                DeadKeyReplacementsSource.Add(new DeadKeyReplacementItem() { From = replacementPair.From, To = replacementPair.To });
            }
            return;
        }

        private void AddNewDeadKeyReplacement(object sender, RoutedEventArgs e)
        {
            DeadKeyReplacementsSource.Add(new DeadKeyReplacementItem()
            {
                From = NewDeadKeyReplacementFromInput.Text,
                To = NewDeadKeyReplacementToInput.Text,
            });
            NewDeadKeyReplacementFromInput.Text = "";
            NewDeadKeyReplacementToInput.Text = "";
        }

        private void PromptDeadKeyConfirm_Click(object sender, RoutedEventArgs e)
        {
            Command =
                new DeadKeyCommand(
                    DeadKeyIndependentTextInput.Text,
                    (from r in DeadKeyReplacementsSource select new ReplacementPair(r.From, r.To)).ToList());
            DialogResult = true;
        }

        #endregion

        
    }
}

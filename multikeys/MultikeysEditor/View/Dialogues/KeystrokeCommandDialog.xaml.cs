using MultikeysEditor.Model;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Unicode;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms;

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
            // focus on the input
            UnicodeTextInput.Focus();

            /* Macros */

            // Initialize the macros dropdown
            // containing options for the user
            // values range from 0x00 to 0xff,
            // not all of them are valid choices, but leave them in anyway.
            KeysConverter kc = new KeysConverter();
            for (int code = 0x00; code <= 0xff; code++)
            {
                // b is virtual key code
                // 1. cast byte to Keys enumeration, the underlying values should match
                // 2. Use the KeysConverter to convert it to string
                // 3. Make a new instance of MacroItem and add it to the dropdown box
                string keyChar = kc.ConvertToString((Keys)code);
                PromptMacrosAddMacroName.Items.Add(new MacroItem { Code = (byte)(code % 0x100), Name = keyChar, DownKeystroke = true });
            }
            PromptMacrosAddMacroName.DisplayMemberPath = "DisplayText";
            PromptMacrosAddMacroName.SelectedValuePath = "Code";
            // It would be interesting to get these values from somewhere else, as a lot of byte values are not valid keystrokes,
            // and some of the names aren't very nice to look at.

            // Populate the other dropdown box
            KeyValuePair<bool, string>[] updown =
            {
                new KeyValuePair<bool, string>(true, Properties.Strings.PromptMacroKeystrokeDown),
                new KeyValuePair<bool, string>(false, Properties.Strings.PromptMacroKeystrokeUp),
            };
            PromptMacrosAddMacroUpDown.ItemsSource = updown;
            PromptMacrosAddMacroUpDown.SelectedValuePath = "Key";
            PromptMacrosAddMacroUpDown.DisplayMemberPath = "Value";

            // Also initialize the collection
            MacroKeystrokesSource = new ObservableCollection<MacroItem>();
            // And initialize the table
            PromptMacrosMainTable.DataContext = this;
            PromptMacrosMainTable.ItemsSource = MacroKeystrokesSource;
            // Generate columns
            PromptMacrosMainTable.Columns.Add(
                new DataGridTextColumn
                {
                    Header = Properties.Strings.PromptMacroColumnHeaderCode,
                    Binding = new System.Windows.Data.Binding("Code"),
                    Width = new DataGridLength(1, DataGridLengthUnitType.Star),
                }
                );

            PromptMacrosMainTable.Columns.Add(
                new DataGridTextColumn
                {
                    Header = Properties.Strings.PromptMacroColumnHeaderName,
                    Binding = new System.Windows.Data.Binding("Name"),
                    Width = new DataGridLength(1, DataGridLengthUnitType.Star),
                }
                );

            PromptMacrosMainTable.Columns.Add(
                new DataGridCheckBoxColumn
                {
                    Header = Properties.Strings.PromptMacroColumnHeaderDown,
                    Binding = new System.Windows.Data.Binding("DownKeystroke"),
                }
                );
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


        /*--- Unicode ---*/
        
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
        

        /*--- Executable ---*/

        private string FileToExecute { get; set; }

        private void ProptExecutableChooseFilename_Click(object sender, RoutedEventArgs e)
        {
            var fileDialog = new OpenFileDialog
            {
                Title = Properties.Strings.ChooseFileToAssignToThisKey,
                Filter = "Executable files|*.exe",
                Multiselect = false,
                CheckPathExists = true,
            };

            if (fileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                FileToExecute = fileDialog.FileName;
                PromptExecutableFilename.Text = fileDialog.FileName;
            }
        }

        private void PromptExecutableArgumentsExpander_Expanded(object sender, RoutedEventArgs e)
        {
            PromptExecutableArguments.Height = 23 * 5;
        }

        private void PromptExecutableArgumentsExpander_Collapsed(object sender, RoutedEventArgs e)
        {
            PromptExecutableArguments.Height = 23;
        }

        private void PromptExecutableConfirm_Click(object sender, RoutedEventArgs e)
        {
            if (string.IsNullOrEmpty(FileToExecute))
            {
                System.Windows.MessageBox.Show(Properties.Strings.WarningNoFileSelected, Properties.Strings.Warning);
                return;
            }
            Command =
                new ExecutableCommand
                {
                    Command = FileToExecute,
                    Arguments = PromptExecutableArguments.Text
                };
            DialogResult = true;
        }

        /*--- Macros ---*/

        // This class is meant to be used both in the dropdown for choosing a new keystroke,
        // and in the table of keystrokes to be written to the new command.
        // In the first case, the downkeystroke property is unused.
        public class MacroItem : INotifyPropertyChanged
        {
            private byte _code;
            public byte Code
            {
                get { return _code; }
                set { _code = value; NotifyPropertyChanged("Code"); }
            }
            private string _name;
            public string Name
            {
                get { return _name; }
                set { _name = value; NotifyPropertyChanged("Name"); }
            }
            private bool _downKeystroke;
            public bool DownKeystroke
            {
                get { return _downKeystroke; }
                set { _downKeystroke = value; NotifyPropertyChanged("DownKeystroke"); }
            }
            // readonly for displaying a user-readable name for an instance of this class.
            public string DisplayText
            {
                get { return string.Format("{0:X2} - {1}", _code, _name); }
            }
            public event PropertyChangedEventHandler PropertyChanged;
            private void NotifyPropertyChanged(string propertyName)
            { PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName)); }

            public MacroItem Copy()
            {
                return new MacroItem
                {
                    Code = _code,
                    Name = _name,
                    DownKeystroke = _downKeystroke,
                };
            }
        }

        // This collection holds the keystrokes to be added to the command
        // and to be displayed on the main list of this tab.
        public ObservableCollection<MacroItem> MacroKeystrokesSource { get; set; }

        // Adds a new keystroke to the list of keystrokes at the end of the list
        private void PromptMacrosAddMacroButtonAdd_Click(object sender, RoutedEventArgs e)
        {
            // Get item from main drowpdown
            MacroItem toAdd = (PromptMacrosAddMacroName.SelectedItem as MacroItem)?.Copy();
            if (toAdd == null) return;

            // Get down/up from secondary dropdown
            bool? isDown = PromptMacrosAddMacroUpDown.SelectedValue as bool?;
            toAdd.DownKeystroke = isDown ?? true;
            MacroKeystrokesSource.Add(toAdd);
        }

        // Adds a new keystroke to the list after the selected item. Do not add if there is no
        // item selected
        private void PromptMacrosButtonAddAfterSelected_Click(object sender, RoutedEventArgs e)
        {
            if (PromptMacrosMainTable.SelectedItem != null)
            {
                MacroItem toAdd = (PromptMacrosAddMacroName.SelectedItem as MacroItem)?.Copy();
                if (toAdd == null) return;

                bool? isDown = PromptMacrosAddMacroUpDown.SelectedValue as bool?;
                toAdd.DownKeystroke = isDown ?? true;

                // Find out the index of the selected item
                int index = PromptMacrosMainTable.SelectedIndex;
                // Then add the new item right after it
                MacroKeystrokesSource.Insert(index: index + 1, item: toAdd);
            }
        }

        // Remove the selected item, if there is a selected item
        private void PromptMacrosButtonDeleteSelected_Click(object sender, RoutedEventArgs e)
        {
            if (PromptMacrosMainTable.SelectedIndex == -1) return;
            MacroKeystrokesSource.Remove(PromptMacrosMainTable.SelectedItem as MacroItem);
        }

        private void PromptMacroButtonConfirm_Click(object sender, RoutedEventArgs e)
        {
            // Create a new command, then exit successfully.
            var listOfKeystrokes = new List<VirtualKeystroke>();

            foreach (var item in MacroKeystrokesSource)
                listOfKeystrokes.Add(
                    new VirtualKeystroke()
                    {
                        VirtualKeyCode = item.Code,
                        KeyDown = item.DownKeystroke,
                    }
                    );

            Command =
                new MacroCommand()
                {
                    TriggerOnRepeat = false,
                    VKeyCodes = listOfKeystrokes,
                };

            // Exits successfully
            DialogResult = true;
            
        }

    }
}

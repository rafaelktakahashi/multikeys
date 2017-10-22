using MultikeysEditor.Model;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Unicode;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace MultikeysEditor.View.Controls
{



    /// <summary>
    /// Interaction logic for CommandPanel.xaml
    /// </summary>
    public partial class CommandPanel : UserControl
    {
        public CommandPanel()
        {
            InitializeComponent();
        }

        /// <summary>
        /// Command that this control represents.
        /// </summary>
        public IKeystrokeCommand Command { get; private set; }

        /// <summary>
        /// Template for creating new lines in the StackPanelData
        /// </summary>
        private TextBlock MakeNewLine(string content)
        {
            var line = this.Resources["Content"] as TextBlock;
            line.Text = content;
            return line;
        }

        private TextBlock MakeNewTitle(string content)
        {
            var title = this.Resources["Title"] as TextBlock;
            title.Text = content;
            return title;
        }


        /// <summary>
        /// Use this method to set this control's command.
        /// This will also update this control's contents.
        /// This method accepts null, representing a key that is not remapped.
        /// </summary>
        public void UpdateCommand(IKeystrokeCommand newCommand)
        {
            StackPanelData.Children.Clear();

            if (newCommand == null)
            {
                WriteSummaryNotRemapped();
            }
            else
            {
                if (newCommand is DeadKeyCommand) { WriteSummaryDeadKey(newCommand as DeadKeyCommand); return; }
                if (newCommand is UnicodeCommand) { WriteSummaryUnicode(newCommand as UnicodeCommand); return; }
                if (newCommand is MacroCommand) { WriteSummaryMacro(newCommand as MacroCommand); return; }
                if (newCommand is ExecutableCommand) { WriteSummaryExecutable(newCommand as ExecutableCommand); return; }
            }
        }

        public void UpdateCommand(Modifier modifier)
        {
            StackPanelData.Children.Clear();

            WriteSummaryModifier(modifier);
        }

        private void WriteSummaryNotRemapped()
        {
            TextBlock notRemappedLine = MakeNewLine(Properties.Strings.ThisKeyIsNotRemapped);
            notRemappedLine.Foreground = Brushes.DarkGray;
            StackPanelData.Children.Add(notRemappedLine);
        }

        private void WriteSummaryDeadKey(DeadKeyCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewTitle(Properties.Strings.ReadableNameDeadkey);
            StackPanelData.Children.Add(titleLine);

            // Independent codepoints:
            TextBlock independentLine = MakeNewLine(newCommand.ContentAsText);
            if (independentLine.Text.Length <= 2) independentLine.FontSize = 20;
            independentLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(independentLine);

            // Name of each
            string text = newCommand.ContentAsText;
            for (int i = 0; i < text.Length; i++)
            {
                int codePoint = char.ConvertToUtf32(text, i);
                if (codePoint > 0xffff) i++;
                TextBlock textLineInfo = MakeNewLine(UnicodeInfo.GetName(codePoint));
                StackPanelData.Children.Add(textLineInfo);
            }

            // Replacements:
            TextBlock replacementsLine = MakeNewLine(Properties.Strings.DeadKeyReplacements);
            StackPanelData.Children.Add(replacementsLine);
            DataGrid dg = new DataGrid()
            {
                CanUserAddRows = false,
                CanUserDeleteRows = false,
                CanUserReorderColumns = false,
                CanUserResizeColumns = false,
                CanUserResizeRows = false,
                CanUserSortColumns = false,
                Focusable = false,
                IsReadOnly = true,
                IsManipulationEnabled = false,
                IsHitTestVisible = false,       // <- user cannot interact
                HeadersVisibility = DataGridHeadersVisibility.None,
                HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled,
                ColumnWidth = new DataGridLength(1, DataGridLengthUnitType.Star),
                HorizontalContentAlignment = HorizontalAlignment.Center,
                CellStyle = new Style(typeof(DataGridCell))
                {
                    Setters = { new Setter(TextBlock.TextAlignmentProperty, TextAlignment.Center) }
                },
                Margin = new Thickness { Left = 10, Top = 5, Right = 10, Bottom = 0 }
            };
            IEnumerable<ReplacementPair> pairs =
                from pair in newCommand.Replacements
                select new ReplacementPair(pair.Key, pair.Value);
            dg.ItemsSource = pairs;
            StackPanelData.Children.Add(dg);

        }

        private void WriteSummaryUnicode(UnicodeCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewTitle(Properties.Strings.ReadableNameUnicodeKey);
            StackPanelData.Children.Add(titleLine);

            // Text
            TextBlock textLine = MakeNewLine(newCommand.ContentAsText);
            if (textLine.Text.Length <= 2) textLine.FontSize = 20;
            textLine.TextWrapping = TextWrapping.Wrap;
            textLine.FontWeight = FontWeights.Bold;
            StackPanelData.Children.Add(textLine);

            // Name of each character (needs improvement, would be nice to show as a table of some sort)
            string text = newCommand.ContentAsText;
            for (int i = 0; i < text.Length; i++)
            {
                int codePoint = char.ConvertToUtf32(text, i);
                if (codePoint > 0xffff) i++;
                TextBlock textLineInfo = MakeNewLine(UnicodeInfo.GetName(codePoint));
                StackPanelData.Children.Add(textLineInfo);
            }

            
        }

        private void WriteSummaryMacro(MacroCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewTitle(Properties.Strings.ReadableNameMacroKey);
            StackPanelData.Children.Add(titleLine);

            // Text
            TextBlock textLine = MakeNewLine("");
            string textLineContent = Properties.Strings.MacroSequence + ":\n";
            foreach (var vkey in newCommand.VKeyCodes)
                textLineContent += vkey.ToString() + "\n";
            textLine.Text = textLineContent;
            StackPanelData.Children.Add(textLine);
        }

        private void WriteSummaryExecutable(ExecutableCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewTitle(Properties.Strings.ReadableNameExecutablekey);
            StackPanelData.Children.Add(titleLine);

            // Command
            TextBlock commandLine = MakeNewLine("Command: " + newCommand.Command);
            commandLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(commandLine);

            // Arguments
            TextBlock argumentsLine = MakeNewLine("Arguments: " + newCommand.Arguments);
            argumentsLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(argumentsLine);
        }

        private void WriteSummaryModifier(Modifier mod)
        {
            // Title
            TextBlock titleLine = MakeNewTitle(Properties.Strings.ReadableNameModifier + ": " + mod.Name);
            StackPanelData.Children.Add(titleLine);
        }

        /// <summary>
        /// For use in a table
        /// </summary>
        private class ReplacementPair
        {
            public ReplacementPair(IList<uint> from, IList<uint> to)
            {
                var sb = new StringBuilder();
                foreach (uint codepoint in from)
                    sb.Append(char.ConvertFromUtf32((int)codepoint));
                From = sb.ToString();

                sb.Clear();
                foreach (uint codepoint in to)
                    sb.Append(char.ConvertFromUtf32((int)codepoint));
                To = sb.ToString();
            }
            public string From { get; set; }
            public string To { get; set; }
        }

    }
}

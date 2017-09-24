using MultikeysGUI.Model;
using System;
using System.Collections.Generic;
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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace MultikeysGUI.View.Controls
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
        private TextBlock MakeNewLine()
        {
            return new TextBlock
            {
                AllowDrop = false,
                FontSize = 18,
                FontWeight = FontWeights.Bold,
                Margin = new Thickness { Left = 10, Top = 5, Right = 10, Bottom = 0 },
                TextTrimming = TextTrimming.CharacterEllipsis
            };
        }


        /// <summary>
        /// Use this method to set this control's command.
        /// This will also update this control's contents.
        /// This method accepts null, representing a key that is not remapped.
        /// </summary>
        /// <param name="newCommand"></param>
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

        private void WriteSummaryNotRemapped()
        {
            TextBlock notRemappedLine = MakeNewLine();
            notRemappedLine.Text = Properties.Strings.ThisKeyIsNotRemapped;
            notRemappedLine.Foreground = Brushes.DarkGray;
            StackPanelData.Children.Add(notRemappedLine);
        }

        private void WriteSummaryDeadKey(DeadKeyCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewLine();
            titleLine.Foreground = Brushes.Blue;
            titleLine.Text = Properties.Strings.ReadableNameDeadkey;
            StackPanelData.Children.Add(titleLine);

            // Independent codepoints:
            TextBlock independentLine = MakeNewLine();
            independentLine.Text = newCommand.ContentAsText;
            if (independentLine.Text.Length <= 2) independentLine.FontSize = 20;
            independentLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(independentLine);

            // Replacements:
            TextBlock replacementsLine = MakeNewLine();
            replacementsLine.Text = Properties.Strings.DeadKeyReplacements;
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
            TextBlock titleLine = MakeNewLine();
            titleLine.Foreground = Brushes.Blue;
            titleLine.Text = Properties.Strings.ReadableNameUnicodeKey;
            StackPanelData.Children.Add(titleLine);

            // Text
            TextBlock textLine = MakeNewLine();
            textLine.Text = newCommand.ContentAsText;
            if (textLine.Text.Length <= 2) textLine.FontSize = 20;
            textLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(textLine);
            
        }

        private void WriteSummaryMacro(MacroCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewLine();
            titleLine.Foreground = Brushes.Blue;
            titleLine.Text = Properties.Strings.ReadableNameMacroKey;
            StackPanelData.Children.Add(titleLine);

            // Text
            TextBlock textLine = MakeNewLine();
            string textLineContent = Properties.Strings.MacroSequence + ":\n";
            foreach (var vkey in newCommand.VKeyCodes)
                textLineContent += vkey.ToString() + "\n";
            textLine.Text = textLineContent;
            StackPanelData.Children.Add(textLine);
        }

        private void WriteSummaryExecutable(ExecutableCommand newCommand)
        {
            // Title
            TextBlock titleLine = MakeNewLine();
            titleLine.Foreground = Brushes.Blue;
            titleLine.Text = Properties.Strings.ReadableNameExecutablekey;
            StackPanelData.Children.Add(titleLine);

            // Command
            TextBlock commandLine = MakeNewLine();
            commandLine.Text = "Command: " + newCommand.Command;
            commandLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(commandLine);

            // Arguments
            TextBlock argumentsLine = MakeNewLine();
            argumentsLine.Text = "Arguments: " + newCommand.Arguments;
            argumentsLine.TextWrapping = TextWrapping.Wrap;
            StackPanelData.Children.Add(argumentsLine);
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

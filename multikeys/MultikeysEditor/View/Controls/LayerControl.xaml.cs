using MultikeysEditor.Domain.Layout;
using MultikeysEditor.Model;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;

namespace MultikeysEditor.View.Controls
{
    /// <summary>
    /// Interaction logic for LayerControl.xaml
    /// </summary>
    public partial class LayerControl : UserControl
    {

        // Comparer class to be used in a dictionary
        class ScancodeComparer : IEqualityComparer<Scancode>
        {
            public bool Equals(Scancode x, Scancode y)
            {
                return x == y;  // operator == is defined for Scancode
            }

            public int GetHashCode(Scancode obj)
            {
                return obj.Code;
            }
        }

        public LayerControl()
        {
            InitializeComponent();

            // The following dictionaries hold the command and the key control on screen
            // that correspond to each scancode.
            // The layout may change at any time, and that change should reflect on the key controls.
            Layout = new Dictionary<Scancode, IKeystrokeCommand>(200, new ScancodeComparer());
            KeyControls = new Dictionary<Scancode, KeyControl>(200, new ScancodeComparer());

            CurrentKey = new Tuple<Scancode, IKeystrokeCommand>(null, null);

            // Empty labels; initialization happens by setting the property.
            Labels = new Dictionary<Scancode, string>(0);

            // Default initializetion
            Labels = LogicalLayoutFactory.GetLogicalLayout("US");
        }


        /// <summary>
        /// Alias of this layer given by the user.
        /// </summary>
        public string Alias { get; set; }

        /// <summary>
        /// The mapping between scancodes and keystroke commands.<para/>
        /// Setting or changing this property does not refresh the view. For that, call RefreshView.
        /// <para/>
        /// Scancodes present in this control but not in Layout are considered not remapped.
        /// <para/>
        /// This property may be set to null in case of a non-remapped layer.
        /// </summary>
        public IDictionary<Scancode, IKeystrokeCommand> Layout { get; private set; }

        /// <summary>
        /// Private dictionary for all KeyControls.
        /// </summary>
        private IDictionary<Scancode, KeyControl> KeyControls { get; set; }

        /// <summary>
        /// Set this property in order to determine the labels that will be printed on each key.
        /// If this property is not set, no labels will be printed on screen.
        /// </summary>
        public IDictionary<Scancode, string> Labels { get; set; }


        /// <summary>
        /// Stores the currently selected key, by scancode and IKeystrokeCommand.
        /// 1. If there is no currently selected key, both items in the tuple are null.
        /// 2. If the currently selected key is not remapped or is a modifier, the second item is null.
        /// </summary>
        public Tuple<Scancode, IKeystrokeCommand> CurrentKey { get; private set; }

        /// <summary>
        /// This property is used by this control to keep track of which keys are currently being used as modifiers.
        /// The value of each modifier is true if that modifier is currently selected.
        /// </summary>
        public IDictionary<Modifier, bool> Modifiers { get; set; }

        /// <summary>
        /// Creates and renders the collection of keys on the screen.<para/>
        /// This method must be called at least once on this control.<para/>
        /// If called again after it's already loaded, this method will not overwrite
        /// any existing data, but certain keys may become inaccessible if the layout is changed.<para/>
        /// Do not call this method to refresh the view; use <code>RefreshView</code> instead.
        /// </summary>
        /// <param name="layout">Physical layout representing the scancode, size and position of each physical key.</param>
        /// <param name="unitLength">Lenght in px used as the unit length.
        /// The resulting keyboard is always 23 units wide and 6.5 units high.</param>
        public void SetLayoutToRender(IPhysicalLayout layout, double unitLength = 45)
        {
            KeyControls.Clear();
            KeysGrid.Children.Clear();
            foreach (var key in layout.Keys)
            {
                // Make a key using the provided information.
                var newKey = MakeKey(key, unitLength);
                // Add the new key to this control. It will be rendered.
                KeysGrid.Children.Add(newKey);
                // Add a handler so that each key fires an event.
                newKey.KeyClicked += new EventHandler(OnAnyKeyControlClicked);
                // Also add the control to the dictionary so that it can be retrieved later
                KeyControls.Add(newKey.Scancode, newKey);
            }
        }


        /// <summary>
        /// Refreshes every key in this view using the current layout and modifiers.
        /// </summary>
        public void RefreshView()
        {
            // This algorithm doesn't look as fast as it could be.
            foreach (var controlPair in KeyControls)
            {
                // Render the key's name (from the logical layout) on the key.
                // If the logical layout contains no label for that key render nothing (an empty string) instead.
                if (Labels.ContainsKey(controlPair.Key))
                { KeyControls[controlPair.Key].SetLabel(Labels[controlPair.Key]); }
                else
                { KeyControls[controlPair.Key].SetLabel(""); }

                // check if it's a modifier; if so, set it
                if (Modifiers.Keys.Any(mod => mod.Scancodes.Contains(controlPair.Key)))
                {
                    KeyControls[controlPair.Key].SetModifier(Modifiers.Keys.First(mod => mod.Scancodes.Contains(controlPair.Key)));

                }
                // get its corresponding command
                else if (Layout.ContainsKey(controlPair.Key))
                {
                    KeyControls[controlPair.Key].SetCommand(Layout[controlPair.Key]);
                }
                else
                {
                    KeyControls[controlPair.Key].SetCommand(null);
                }
            }
        }
        

        /// <summary>
        /// Refreshes every key in this view according to the commands in the specified layout and modifiers.<para/>
        /// </summary>
        public void RefreshView(IDictionary<Scancode, IKeystrokeCommand> commands,
            ICollection<Modifier> modifiers)
        {
            // When setting the new commands, we must specify the custom comparator:
            Layout = new Dictionary<Scancode, IKeystrokeCommand>(commands, new ScancodeComparer());

            Modifiers = new Dictionary<Modifier, bool>();
            foreach (var mod in modifiers)
                Modifiers.Add(mod, false);

            RefreshView();
        }

        /// <summary>
        /// Refreshes the specified key in this view according to the specified command.
        /// </summary>
        public void RefreshView(Scancode key, IKeystrokeCommand command)
        {
            Layout[key] = command;
            // Notify the key control about this change, as to render the new command.
            KeyControls[key].SetCommand(command);
        }

        /// <summary>
        /// Looks for each key in the modifier, and updates it.
        /// </summary>
        public void RefreshView(Modifier modifier, bool isPressed)
        {
            foreach (var scancode in modifier.Scancodes)
            {
                // notify each relevant key control;
                KeyControls[scancode].IsModifierSelected = isPressed;
                KeyControls[scancode].SetModifier(modifier);
            }
        }

        /// <summary>
        /// Make a new KeyControl that's ready to be rendered, based on the information provided.
        /// The resulting object contains information about its size and position on screen.
        /// </summary>
        /// <param name="key">Object representing scancode, size and position of a physical key.</param>
        /// <param name="unitLength">Length, in pixels, to be used as the unit length for size and position.</param>
        /// <returns></returns>
        private KeyControl MakeKey(PhysicalKey key, double unitLength)
        {
            return new KeyControl()
            {
                Scancode = key.Scancode,
                Width = key.Width * unitLength,
                Height = key.Height * unitLength,
                VerticalAlignment = VerticalAlignment.Top,
                HorizontalAlignment = HorizontalAlignment.Left,     // its position in the control is set through Margin
                Margin = new Thickness(key.XOffset * unitLength, key.YOffset * unitLength, 0, 0),
                Shape = key.Shape,
            };
        }
        
        #region Events

        public event EventHandler KeyClicked;

        // Occurs when any key control in this layer control is clicked.
        private void OnAnyKeyControlClicked(object sender, EventArgs e)
        {
            // Update the currently selected key.
            var scancode = (sender as KeyControl).Scancode;
            if (Layout.ContainsKey(scancode))
            {
                // If key is mapped to something
                CurrentKey = new Tuple<Scancode, IKeystrokeCommand>(scancode, Layout[scancode]);
            }
            else
            {
                // If key is not mapped, or is a modifier
                CurrentKey = new Tuple<Scancode, IKeystrokeCommand>(scancode, null);
            }

            // Bubble up the event to the keyboard control.
            KeyClicked?.Invoke(sender, e);
        }

        #endregion

    }

}

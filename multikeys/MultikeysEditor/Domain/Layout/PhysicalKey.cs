using MultikeysEditor.Model;

namespace MultikeysEditor.Domain.Layout
{
    /// <summary>
    /// Represents a single physical key in a keyboard;
    /// These are used to describe a physical layout.
    /// </summary>
    public class PhysicalKey
    {
        public PhysicalKey(Scancode sc, double width, double height, double xOffset, double yOffset, PhysicalKeyShape shape = PhysicalKeyShape.Rectangular)
        {
            Scancode = sc;
            Width = width;
            Height = height;
            XOffset = xOffset;
            YOffset = yOffset;
            Shape = shape;
        }

        public PhysicalKey(string sc, double width, double height, double xOffset, double yOffset, PhysicalKeyShape shape = PhysicalKeyShape.Rectangular)
        {
            Scancode = Scancode.FromString(sc);
            Width = width;
            Height = height;
            XOffset = xOffset;
            YOffset = yOffset;
            Shape = shape;
        }

        /// <summary>
        /// Represents the physical signal sent by pressing this key on the keyboard.
        /// </summary>
        public Scancode Scancode { get; set; }

        /// <summary>
        /// Lenght of this key, in units.
        /// </summary>
        public double Width { get; set; }

        /// <summary>
        /// Height of this key, in units.
        /// </summary>
        public double Height { get; set; }

        /// <summary>
        /// Distance from the left edge, in units.
        /// </summary>
        public double XOffset { get; set; }

        /// <summary>
        /// Distance from the top edge, in units.
        /// </summary>
        public double YOffset { get; set; }

        /// <summary>
        /// Shape of this key.
        /// </summary>
        public PhysicalKeyShape Shape { get; set; }
    }
}

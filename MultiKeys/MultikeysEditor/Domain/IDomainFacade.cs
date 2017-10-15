using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using MultikeysEditor.Domain.Layout;
using MultikeysEditor.Model;

namespace MultikeysEditor.Domain
{
    /// <summary>
    /// Interface that defines the domain facade.
    /// </summary>
    interface IDomainFacade
    {
        /// <summary>
        /// Save a layout into an xml file.
        /// </summary>
        /// <param name="model">Object containing the model</param>
        /// <param name="path">Fully qualified path to file where the layout will be created or overriden.</param>
        void SaveLayout(MultikeysLayout model, string path);

        /// <summary>
        /// Read a layout from an xml file.
        /// </summary>
        /// <param name="path">Fully qualified path to file to be read.</param>
        /// <returns>An object containing the model</returns>
        MultikeysLayout LoadLayout(string path);

        /// <summary>
        /// Get an instance that represents a collection of physical keys according to a standard.
        /// </summary>
        /// <param name="standard">Physical layout standard to be used when creating the physical layout</param>
        /// <returns>An object representing the scancode, size and position of each key in a physical layout</returns>
        IPhysicalLayout GetPhysicalLayout(PhysicalLayoutStandard standard);

        /// <summary>
        /// Prompt the user to press any key on a keyboard, then returns the unique name of that keyboard.
        /// Implementations of this method are implied to be async.
        /// </summary>
        /// <returns></returns>
        Task<string> DetectKeyboardUniqueName();

    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using MultikeysGUI.Domain.Layout;
using MultikeysGUI.Model;
using MultikeysGUI.Persistence;

namespace MultikeysGUI.Domain
{
    /// <summary>
    /// Implementation of the domain facade.
    /// </summary>
    class DomainFacade : IDomainFacade
    {
        /// <throws>XmlSchemValidationException</throws>
        public MultikeysLayout LoadLayout(string path)
        {
            return XmlPersistence.Load(path);
        }

        public void SaveLayout(MultikeysLayout model, string path)
        {
            // TODO: Apply validation rules, such as not allowing a <keyboard> without <layer>s.
            XmlPersistence.Save(model, path);
        }

        public IPhysicalLayout GetPhysicalLayout(PhysicalLayoutStandard standard, bool useBigReturn = false)
        {
            return PhysicalLayoutFactory.FromStandard(standard, useBigReturn);
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultikeysGUI.Domain
{
    /// <summary>
    /// Singleton for a facade.
    /// </summary>
    class DomainFacade
    {
        private DomainFacade() { }

        private static DomainFacade _instance = null;

        public static DomainFacade Instance
        {
            get
            {
                if (_instance == null) _instance = new DomainFacade();
                return _instance;
            }
        }
    }
}

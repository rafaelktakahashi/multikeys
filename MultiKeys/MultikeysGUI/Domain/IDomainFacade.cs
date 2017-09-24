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
    /// Interface that defines the domain facade.
    /// </summary>
    interface IDomainFacade
    {

        void SaveLayout(MultikeysRoot model, string path);
        MultikeysRoot LoadLayout(string path);



    }
}

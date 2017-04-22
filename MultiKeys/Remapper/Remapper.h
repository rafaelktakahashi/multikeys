#pragma once

#include "RemapperAPI.h"
#include "KeystrokeCommands.h"
#include "Keyboard.h"

// method readSettings() implemented in a separate cpp.

namespace Multikeys
{

	class Remapper : public IRemapper
	{
	private:

		mutable Scancode workScancode;
		std::vector<PKeyboard> keyboards;

	public:
		Remapper()
		{}

		// This will fill the keyboard vector with pointers to allocated keyboards.
		// Implemented in XmlParser.cpp
		bool loadSettings(const std::wstring filename) override;

		bool evaluateKey(
			RAWKEYBOARD* const keypressed,
			WCHAR* const deviceName,
			OUT PKeystrokeCommand* const out_action) const override
		{
			// Check each keyboard until name matches,
			// then call its method for checking a key
			for (auto it = keyboards.begin();
				it != keyboards.end();
				it++)
			{
				if ( wcscmp(deviceName, (*it)->deviceName.c_str()) )
				{
					this->workScancode.flgE0 = (keypressed->Flags & RI_KEY_E0) & RI_KEY_E0;
					this->workScancode.flgE1 = (keypressed->Flags & RI_KEY_E1) & RI_KEY_E1;
					this->workScancode.makeCode = keypressed->MakeCode;
					return ((*it)->evaluateKey(this->workScancode, keypressed->VKey,
						(keypressed->Flags & RI_KEY_BREAK) == RI_KEY_BREAK, out_action)
						);
				}
			}
			// If no keyboard matches, there's no remap and input shouldn't be blocked:
			return false;
		}

		~Remapper() override
		{
			for (auto it = keyboards.begin();
				it != keyboards.end();
				it++)
			{
				// Dereference iterator to get a PKeyboard
				// Delete PKeyboard (which is a Keyboard pointer)
				delete (*it);
			}
		}
	};



}


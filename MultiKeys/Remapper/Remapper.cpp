#include "stdafx.h"

// Implementation of Remapper methods
#include "Remapper.h"

namespace Multikeys
{

	// Pure virtual destructors need an implementation.
	IRemapper::~IRemapper() { }

	Remapper::Remapper() { }

	bool Remapper::evaluateKey(
		// Type RAWKEYBOARD is from the WinAPI
		RAWKEYBOARD* const keypressed,
		wchar_t* const deviceName,
		OUT PKeystrokeCommand* const out_action)
	{
		// Check each keyboard until name matches,
		// then call its method for checking a key
		for (auto it = keyboards.begin();
			it != keyboards.end();
			it++)
		{
			
			// If keyboard name matches:
			/*
			* Update: Since an empty string is used to represent "remap any non-remapped keyboard",
			* we check for that too. It's important that this keyboard with an empty string as name
			* be the last in the list.
			*/
			if (wcscmp(deviceName, (*it)->deviceName.c_str()) == 0
				|| wcscmp(L"", (*it)->deviceName.c_str()) == 0)
			{
				this->workScancode.flgE0 = keypressed->Flags & RI_KEY_E0;
				this->workScancode.flgE1 = keypressed->Flags & RI_KEY_E1;
				this->workScancode.makeCode = keypressed->MakeCode & 0xff;
				return (
						(*it)->evaluateKey(this->workScancode,
								keypressed->VKey & 0xff,
								(keypressed->Flags & RI_KEY_BREAK) == RI_KEY_BREAK,
								out_action)
					);
			}
		}
		// If no keyboard matches, there's no remap and input shouldn't be blocked:
		return false;
	}

	Remapper::~Remapper()
	{
		for (auto it = keyboards.begin();
			it != keyboards.end();
			it++)
		{
			// Dereference iterator to get a Keyboard*
			// Delete Keyboard*
			delete (*it);
		}
	}

	void Create(OUT PRemapper* instance)
	{
		*instance = new Remapper();
	}

	void Destroy(PRemapper* instance)
	{
		delete (*instance);
		*instance = nullptr;
	}

}
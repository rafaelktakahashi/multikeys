#pragma once

#include "stdafx.h"


#define VIRTUAL_MODIFIER_LCTRL		0x80
#define VIRTUAL_MODIFIER_RCTRL		0x40
#define VIRTUAL_MODIFIER_LALT		0x20
#define VIRTUAL_MODIFIER_RALT		0x10
#define VIRTUAL_MODIFIER_LWIN		0x08
#define VIRTUAL_MODIFIER_RWIN		0x04
#define VIRTUAL_MODIFIER_LSHIFT		0x02
#define VIRTUAL_MODIFIER_RSHIFT		0x01


struct Level
{
	BYTE modifiers;			// combination of modifiers that identify this level
							// unrelated to virtual modifiers used to simulate output
							// (we should get rid of those, actually, they do the same as macros)
	std::unordered_map<KEYSTROKE_INPUT, IKeystrokeOutput*> layout;

	Level() : modifiers(0)
	{}
	Level(BYTE _modifiers) : modifiers(_modifiers)
	{}
};

// Only the remapper and the parser need to know about this.
// Used to internally store a group of remaps associated to a specific keyboard
struct KEYBOARD
{

	// NEW:
	std::wstring deviceName;

	// high bit means extended key, last byte is the vkey code
	WORD modifierVKeyCodes[8];		// initialize to all zeroes

	IKeystrokeOutput * noAction;	// initialize in constructor - corresponds to no action

	std::vector<Level> levels;

	BYTE modifierState;

	Level * activeLevel;

	KEYBOARD() : noAction(new NoOutput()), modifierState(0), activeLevel(nullptr)
	{
		for (int i = 0; i < 8; i++)
			modifierVKeyCodes[i] = 0;
	}




	BOOL evaluateKeystroke(KEYSTROKE_INPUT * keypressed, OUT IKeystrokeOutput ** out_action)
	{
		// Before responding, check if it's a modifier
		for (unsigned short i = 0, e = 1;
			i < 8;
			i++, e <<= 1)		// i increments, e bitshifts left
		{
			if (modifierVKeyCodes[i] == 0) break;

			if (	// last byte (0xff) is virtual key; first bit (0x8000) is extended key
				(modifierVKeyCodes[i] & 0xff) == keypressed->virtualKey &&
				(modifierVKeyCodes[i] & 0x8000) == ((keypressed->flags & RI_KEY_E0) > 0)
				)
			{
				// found the match, variable e currently holds the (i + 1)-th bit on 
				// (for example, in the third iteration it will be 0000 0100)
				if ((keypressed->flags & RI_KEY_BREAK) > 0)
				{
					// is keyup - deactivate flag in modifierState
					if (modifierState & e)
						modifierState -= e;
				}
				else
				{
					// is keydown - set flag in modifierState
					modifierState |= e;
				}

				// Update the active level according to modifier state
				activeLevel = nullptr;
				for (auto iterator = levels.begin(); iterator != levels.end(); iterator++)
				{
					if (iterator->modifiers == modifierState)
					{
						activeLevel = &(*iterator);		// Iterators are not pointers
						break;
					}
				}
				// If no matching level was found, it's null.

				*out_action = noAction;	// there is no action associated with this keystroke
				return TRUE;			// but block it nonetheless
			}
			// didn't match; next
		}
		// none matched; not modifier


		// If current modifier state corresponds to no level, it's null and no keys are blocked:
		if (activeLevel == nullptr)
			return FALSE;

		{
			WCHAR* buffer = new WCHAR[200];
			swprintf_s(buffer, 200, L"Keyboard: looking for sc%x vkey%x ext%d\n",
				keypressed->scancode, keypressed->virtualKey, ((keypressed->flags & RI_KEY_E0) ? 1 : 0));
			OutputDebugString(buffer);
			delete[] buffer;
		}
		keypressed->flags = 0;		// erase flags because the map doesn't include them
		keypressed->virtualKey = 0;		// do not compare using virtual key

		// Goddammit we should just map from scancodes and an extended flag instead
		// of a bunch of stuff that we have to manually remove

		// Look in currently active level
		auto iterator = activeLevel->layout.find(*keypressed);
		if (iterator != activeLevel->layout.end())
		{
			*out_action = iterator->second;		// copy the pointer to IKeystrokeOutput
			return TRUE;	// block the key
		}
		return FALSE;		// do not block the key
	}

	VOID resetModifierState()
	{
		activeLevel = nullptr;
		for (auto iterator = levels.begin(); iterator != levels.end(); iterator++)
		{
			if (iterator->modifiers == 0)
				activeLevel = &(*iterator);
		}
		modifierState = 0;
	}

	

	
};


namespace Multikeys
{
	
	class Remapper
	{
	private:
		// work variable
		WCHAR* wcharWork;


		// a vector of keyboards
		std::vector<KEYBOARD> keyboards;
		// to hold each keyboard
		// It would be possible to make a map between keyboard name and keyboard
		// but we won't have that many at once to require such a thing

		

		// Class that will simulate the output for this remapper
		// InputSimulator inputSimulator;

	public:
		
		// Constructor
		// Parameters:
		//		std::string - full path to the configuration file to be read
		// Remapper(std::string filename);
		// Remapper(std::wstring filename);
		// Constructor
		//
		// Creates an instance with an empty map. Must be properly initialized with LoadSettings
		//
		// Parameters: none
		Remapper();

		// Loads the settings in file at filename into memory
		BOOL LoadSettings(std::string filename);
		BOOL LoadSettings(std::wstring filename);

		// EvaluateKey
		//
		// Receives information about a keypress, analyses it, then if there is a remap,
		//		returns information about the remapped input.
		//
		// Parameters:
		//		RAWKEYBOARD* keypressed - pointer to the structure representing the keyboard input,
		//				from the Raw Input API.
		//		WCHAR* deviceName - wide string containing the name of the device that generated the keystroke
		//		IKeystrokeOutput** out_action - if there is a remap for the keystroke in RAWKEYBOARD* keypressed
		//				for the keyboard of name WCHAR* deviceName, then this will point to a
		//				IKeystrokeOutput pointer pointing to the output to be simulated.
		// Return value:
		//		TRUE - There is a remap, and it's been placed in *out_action
		//		FALSE - There is no remap for this key; do not block key
		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, IKeystrokeOutput** out_action);





	};






	

	
}





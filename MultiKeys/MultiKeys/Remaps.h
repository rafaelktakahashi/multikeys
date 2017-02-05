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
	// DWORD: last two bytes (low WORD) contains the scancode, while the first two bytes (high WORD)
	// contains the prefix (0xe0 or 0xe1)
	std::unordered_map<DWORD, IKeystrokeOutput*> layout;

	Level() : modifiers(0)
	{}
	Level(BYTE _modifiers) : modifiers(_modifiers)
	{}


	// State of modifiers, in the same order as its keyboard, that identify this level
	// 0 - modifier needs to be off
	// 1 - modifier needs to be on
	// 2 - only one of these modifiers need to be on (be careful with ambiguous levels)
	BYTE modifiers2[8];

	VOID setModifiers2(BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6, BYTE b7, BYTE b8)
	{
		modifiers2[0] = b1;		modifiers2[1] = b2;
		modifiers2[2] = b3;		modifiers2[3] = b4;
		modifiers2[4] = b5;		modifiers2[5] = b6;
		modifiers2[6] = b7;		modifiers2[7] = b8;
	}

	// Returns true if _modifiers should trigger this level
	BOOL isEqualTo(const BYTE _modifiers)
	{
		BOOL twoRequirement = FALSE;
		for (short i = 0; i < 8; i++)
		{
			switch (modifiers2[i])
			{
			case 0:
				if ((_modifiers << i) == 1) return FALSE;
				break;
			case 1:
				if ((_modifiers << i) == 0) return FALSE;
			case 2:
				if ((_modifiers << i) == 1) twoRequirement = TRUE;
			}
		}
		// if there were no "2", return true:
		for (short i = 0; i < 8; i++)
		{
			if (modifiers2[i] == 2) return twoRequirement;
		}
		return TRUE;
	}

};

// Only the remapper and the parser need to know about this.
// Used to internally store a group of remaps associated to a specific keyboard
struct KEYBOARD
{

private:

	// Returns true when keypress was a modifier
	BOOL _updateKeyboardState(USHORT scancode, USHORT vKeyCode, BOOL flag_E0, BOOL flag_E1, BOOL flag_keyup)
	{
		for (unsigned short i = 0, e = 1;
			i < 8;
			i++, e <<= 1)		// i increments, e bitshifts left
		{
			if (modifierVKeyCodes[i] == 0) break;

			if (	// last byte (0xff) is virtual key; first bit (0x8000) is extended key
				(modifierVKeyCodes[i] & 0xff) == vKeyCode &&
				(modifierVKeyCodes[i] & 0x8000) == flag_E0
				)
			{
				// found the match, variable e currently holds the (i + 1)-th bit on 
				// (for example, in the third iteration it will be 0000 0100)
				if (flag_keyup)
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
				// If no matching level was found, it remains null.
				return TRUE;	// because match was found
			}
			// didn't match; next
		}
		// none matched; not modifier
		return FALSE;
	}

public:

	
	std::wstring deviceName;

	// high bit means extended key, last byte is the vkey code
	// Extended key is necessary in case of the arrow keys and the six keys above them
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


	BOOL evaluateKeystroke(USHORT scancode, USHORT vKeyCode, BOOL flag_E0, BOOL flag_E1, BOOL flag_keyup, OUT IKeystrokeOutput ** out_action)
	{
		// Before responding, check if it's a modifier
		if (_updateKeyboardState(scancode, vKeyCode, flag_E0, flag_E1, flag_keyup))
		{
			// If this key is a modifier
			*out_action = noAction;		// then there is no action associated with this keystroke
			return TRUE;				// however, we must still block it.
		}

		// none matched; not modifier


		// If current modifier state corresponds to no level, it's null and no keys are blocked:
		if (activeLevel == nullptr)
			return FALSE;

		{
			WCHAR* buffer = new WCHAR[200];
			swprintf_s(buffer, 200, L"Keyboard: looking for sc%x vkey%x ext%d\n",
				scancode, vKeyCode, flag_E0);
			OutputDebugString(buffer);
			delete[] buffer;
		}

		// make the DWORD key: least significant 16 bits are the scancode;
		//						most significant 16 bits are the prefix (0xe0 or 0xe1)
		DWORD key = scancode;
		if (flag_E0)
			key |= (0xe0 << 16);
		else if (flag_E1)
			key |= (0xe1 << 16);


		// Look in currently active level
		auto iterator = activeLevel->layout.find(key);
		if (iterator != activeLevel->layout.end())
		{
			*out_action = iterator->second;		// copy the pointer to IKeystrokeOutput
			return TRUE;	// block the key
		}
		return FALSE;		// do not block the key
	}


	BOOL addModifier(USHORT virtualKeyCode)
	{
		for (int i = 0; i < 8; i++)
		{
			if (modifierVKeyCodes[i] == 0)
			{
				// found an empty spot
				modifierVKeyCodes[i] = virtualKeyCode;
				return TRUE;
			}
		}
		// never found a spot
		return FALSE;
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





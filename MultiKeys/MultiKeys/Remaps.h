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
	
							// unrelated to virtual modifiers used to simulate output
	// DWORD: last two bytes (low WORD) contains the scancode, while the first two bytes (high WORD)
	// contains the prefix (0xe0 or 0xe1)
	std::unordered_map<DWORD, IKeystrokeOutput*> layout;



	// State of modifiers, in the same order as its keyboard, that identify this level
	// 0 - modifier needs to be off
	// 1 - modifier needs to be on
	// 2 - only one of these modifiers need to be on (be careful with ambiguous levels)
	BYTE modifiers[8];

	VOID setModifiers2(
		BYTE b1 = 0, BYTE b2 = 0,
		BYTE b3 = 0, BYTE b4 = 0,
		BYTE b5 = 0, BYTE b6 = 0,
		BYTE b7 = 0, BYTE b8 = 0)
	{
		modifiers[0] = b1;		modifiers[1] = b2;
		modifiers[2] = b3;		modifiers[3] = b4;
		modifiers[4] = b5;		modifiers[5] = b6;
		modifiers[6] = b7;		modifiers[7] = b8;
	}

	// Returns true if _modifiers should trigger this level
	BOOL isEqualTo(BYTE _modifiers)
	{
		BOOL twoRequirement = FALSE;
		for (short i = 0, e = 1; i < 8; i++, e <<= 1)
		{
			switch (modifiers[i])
			{
			case 0:
				if (_modifiers & e) return FALSE;
				break;
			case 1:
				if (!(_modifiers & e)) return FALSE;
				break;
			case 2:
				if (_modifiers & e) twoRequirement = TRUE;
				break;
			}
		}
		// if there were no "2", return true:
		for (short i = 0; i < 8; i++)
		{
			if (modifiers[i] == 2) return twoRequirement;
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
				((modifierVKeyCodes[i] & 0x8000) > 0) == flag_E0
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
					// if (iterator->modifiers == modifierState)
					if (iterator->isEqualTo(modifierState))
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

	// Name of this device, in wide string because the Raw Input API returns a wide string
	std::wstring deviceName;

	// Virtual-key codes of each registered modifier for this keyboard, 0 for none
	// high bit means extended key, last byte is the vkey code
	// Extended key is necessary in case of the arrow keys and the six keys above them
	WORD modifierVKeyCodes[8];		// initialize to all zeroes

	IKeystrokeOutput * noAction;	// initialize in constructor - corresponds to no action
									// Declared to be readily available to return a dummy action

	// Each set of remaps for each combination of modifiers
	std::vector<Level> levels;

	// Combination of currently active (physically pressed) modifiers; each bit is a set modifier,
	// in the same order as modifierVKeyCodes.
	BYTE modifierState;

	// Current mapping; NULL if current combination of modifiers corresponds to no level
	Level * activeLevel;

	// Pointer to dead key waiting the next character; NULL if no dead key is active
	DeadKeyOutput * activeDeadKey;

	// Constructor
	KEYBOARD() : noAction(new NoOutput()), modifierState(0), activeLevel(nullptr)
	{
		for (int i = 0; i < 8; i++)
			modifierVKeyCodes[i] = 0;

		activeDeadKey = nullptr;
	}


	// Receives information about a keypress, and returns true if the keystroke should be blocked.
	// If the key should be blocked, pointer pointer by IKeystrokeOutput** points to an action that
	// should be executed (that command may or may not execute an action).
	BOOL evaluateKeystroke(USHORT scancode, USHORT vKeyCode, BOOL flag_E0, BOOL flag_E1, BOOL flag_keyup,
							OUT IKeystrokeOutput ** out_action)
	{
		// For the purposes of checking modifiers, we need the specific scancodes
		// for left and right variants of Shift, Ctrl and Alt
		{	// Transform generic into variant-specific
			USHORT LRvKey = vKeyCode;
			if (vKeyCode == VK_SHIFT && scancode == 0x2a)
				LRvKey = VK_LSHIFT;
			else if (vKeyCode == VK_SHIFT && scancode == 0x36)
				LRvKey = VK_RSHIFT;
			else if (vKeyCode == VK_CONTROL)
				LRvKey = (flag_E0 ? VK_RCONTROL : VK_LCONTROL);
			else if (vKeyCode == VK_MENU)
				LRvKey = (flag_E0 ? VK_RMENU : VK_LMENU);			// MENU is Alt key

			// Before responding, check if it's a modifier
			if (_updateKeyboardState(scancode, LRvKey, flag_E0, flag_E1, flag_keyup))
			{
				// If this key is a modifier
				*out_action = noAction;		// then there is no action associated with this keystroke
				return TRUE;				// however, we must still block it.
			}
		}

		// none matched; not modifier


		// If current modifier state corresponds to no mapping, it's null and no keys are blocked:
		if (activeLevel == nullptr)
			return FALSE;

	#if DEBUG
		{
			WCHAR* buffer = new WCHAR[200];
			swprintf_s(buffer, 200, L"Keyboard: looking for sc%x vkey%x ext%d\n",
				scancode, vKeyCode, flag_E0);
			OutputDebugString(buffer);
			delete[] buffer;
		}
	#endif

		// make the DWORD key: least significant 16 bits are the scancode;
		//						most significant 16 bits are the prefix (0xe0 or 0xe1)
		DWORD key = scancode;
		if (flag_E0)
			key |= (0xe0 << 16);
		else if (flag_E1)
			key |= (0xe1 << 16);


		BOOL returnValue;
		// Look in currently active level
		auto iterator = activeLevel->layout.find(key);
		if (iterator != activeLevel->layout.end())
		{
			*out_action = iterator->second;		// copy the pointer to IKeystrokeOutput
			returnValue = TRUE;	// block the key
		}
		returnValue = FALSE;		// do not block the key

		// If there is an active dead key: this must go through it.
		// If there is no active dead key: check if this is a dead key, and if it is,
		//								have the active dead key pointer point here,
		//								and return a dummy action.

		return returnValue;
	}


	BOOL addModifier(USHORT virtualKeyCode, BOOL isExtended)
	{
		for (int i = 0; i < 8; i++)
		{
			if (modifierVKeyCodes[i] == 0)
			{
				// found an empty spot
				modifierVKeyCodes[i] = virtualKeyCode;
				if (isExtended)
					modifierVKeyCodes[i] |= 0x8000;
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
			// if (iterator->modifiers == 0)
			if (iterator->isEqualTo(0))
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





#pragma once

#include "stdafx.h"
#include "Keystrokes.h"




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

	VOID setModifiers(
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
			// I wish trits were a thing
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

	// Prefix must be bitshifted left 16 bits
	VOID insertPair(DWORD prefixedScancode, IKeystrokeOutput* command)
	{
		layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(prefixedScancode, command));
	}

	VOID insertPair(WORD scancode, IKeystrokeOutput* command)
	{
		layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(scancode, command));
	}

	VOID insertPair(WORD scancodePrefix, WORD scancode, IKeystrokeOutput* command)
	{
		layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(scancode | (scancodePrefix << 16), command));
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

			// Look for the modifier that matched the received keystroke
			if (	// last byte (0xff) is virtual key; first bit (0x8000) is extended flag
				(modifierVKeyCodes[i] & 0xff) == vKeyCode &&
				((modifierVKeyCodes[i] & 0x8000) > 0) == flag_E0
				)
			{
				// found the modifier that changed, variable e currently holds the (i + 1)-th bit on 
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
#if DEBUG
						OutputDebugString(L"Shifting level!\n");
#endif
						break;
					}
				}
				// If no matching level was found, it remains null.
#if DEBUG
				if (activeLevel == nullptr)
				{
					OutputDebugString(L"Current level is null!\n");
				}
#endif
				return TRUE;	// because match was found
			}
			// didn't match; next
		}
		// none matched; not modifier
		return FALSE;
	}

	// utility function to evaluate, roughly, whether a virtual-key code is printable
	BOOL vKeyIsPrintable(USHORT vKey)
	{
		// These ranges are not infallible. Correct if necessary.
		// If in doubt, check a virtual key table
		return (
			(vKey >= 0x30 && vKey <= 0x5a)
			||
			(vKey >= 0x60 && vKey <= 0x6f)
			||
			(vKey >= 0xba && vKey <= 0xe4)
			||
			vKey == VK_RETURN
			);
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
	KEYBOARD() : noAction(new NoOutput()), modifierState(0), activeLevel(nullptr), activeDeadKey(nullptr)
	{
		for (int i = 0; i < 8; i++)
			modifierVKeyCodes[i] = 0;

		activeDeadKey = nullptr;
	}


	// Receives information about a keypress, and returns true if the keystroke should be blocked.
	// USHORT scancode - scancode for the keypress to be evaluated
	// USHORT vKeyCode - virtual key code for the keypress to be evaluated
	// BOOL flag_E0 - TRUE if the scancode has the 0xe0 prefix
	// BOOL flag_E1 - TRUE if the scancode has the 0xe1 prefix
	// BOOL flag_keyup - TRUE if the keypress is a keypress up
	// OUT IKeystrokeOutput**const out_action - pointer to an IKeystrokeOutput*; if this function
	//		returns TRUE, that pointer will become a pointer to the remapped command.
	BOOL evaluateKeystroke(USHORT scancode, USHORT vKeyCode, BOOL flag_E0, BOOL flag_E1, BOOL flag_keyup,
							OUT IKeystrokeOutput**const out_action)
	{
		// For the purposes of checking modifiers, we need the specific virtual key codes
		// for left and right variants of Shift, Ctrl and Alt
		{	// brackets for locality
			USHORT LRvKey = vKeyCode;
			BOOL newFlag_E0 = flag_E0;
			if (vKeyCode == VK_SHIFT && scancode == 0x2a)
				LRvKey = VK_LSHIFT;
			else if (vKeyCode == VK_SHIFT && scancode == 0x36)
				LRvKey = VK_RSHIFT;
			else if (vKeyCode == VK_CONTROL)
			{
				LRvKey = (flag_E0 ? VK_RCONTROL : VK_LCONTROL);
				newFlag_E0 = 0;
			}
			else if (vKeyCode == VK_MENU)
			{
				LRvKey = (flag_E0 ? VK_RMENU : VK_LMENU);
				newFlag_E0 = 0;
			}			// MENU is Alt key

			// First thing, check if it's a modifier
			if (_updateKeyboardState(scancode, LRvKey, newFlag_E0, flag_E1, flag_keyup))
			{
#if DEBUG
				OutputDebugString(L"Keyboard: Identified a modifier keystroke.\n");
#endif
				// If this key is a modifier
				*out_action = noAction;		// then there is no action associated with this keystroke
				return TRUE;				// however, we must still block it.
			}
		}

		// none matched; not modifier


	#if DEBUG
		WCHAR* buffer = new WCHAR[200];
		swprintf_s(buffer, 200, L"Keyboard: looking for sc%x vkey%x ext%d\n",
			scancode, vKeyCode, flag_E0);
		OutputDebugString(buffer);
		delete[] buffer;
	#endif

		// If current level is non-empty, we must find the key
		BOOL remapExists = FALSE;
		IKeystrokeOutput* remappedCommand;

		if (activeLevel != nullptr)
		{
			// must find a remap if there is any

			// make the DWORD key: least significant 16 bits are the scancode;
			//						most significant 16 bits are the prefix (0xe0 or 0xe1)
			DWORD key = scancode;
			if (flag_E0)
				key |= (0xe0 << 16);
			else if (flag_E1)
				key |= (0xe1 << 16);

			// look in currently active level
			auto iterator = activeLevel->layout.find(key);
			if (iterator != activeLevel->layout.end())
			{
				remapExists = TRUE;
				remappedCommand = iterator->second;
			}
		}

		// Before even this part
		// If the new keypress is a keyup, do not trigger any dead key,
		//		nor set this as a dead key. Just send it.
		if (flag_keyup)
		{
			if (remapExists)
				*out_action = remappedCommand;
			// return whether a remap was found
			return remapExists;
		}

		// is dead key active?
		if (activeDeadKey != nullptr)
		{
			// if the new input is a keystroke that generates a character,
			// XOR the new input is remapped to a unicode output
			//		then the new input is given to the dead key, and the dead
			//		key is returned
			if (remapExists)
			{
				if (remappedCommand->getType() == KeystrokeOutputType::UnicodeOutput
					|| remappedCommand->getType() == KeystrokeOutputType::DeadKeyOutput)
				{
					activeDeadKey->setNextCommand(remappedCommand);
					*out_action = activeDeadKey;
					activeDeadKey = nullptr;
					return TRUE;
				}
			}
			else		// !remapExists
			{
				if (vKeyIsPrintable(vKeyCode))
				{
					activeDeadKey->setNextCommand(vKeyCode);
					*out_action = activeDeadKey;
					activeDeadKey = nullptr;
					return TRUE;
				}
			}
		}
		else
		{
			// no dead key is active
			// if new input maps to a dead key,
			//		then new input is set as active dead key
			if (remapExists)
				if (remappedCommand->getType() == KeystrokeOutputType::DeadKeyOutput)
				{
					activeDeadKey = (DeadKeyOutput*)remappedCommand;
					*out_action = noAction;
					return TRUE;
				}
		}


		// At this point:
		// 1: new input is not a printable virtual key code
		// 2: new input is not mapped to a unicode output
		// 3: new input is not a dead key itself
		// place remapped command in the out parameter if there was any
		if (remapExists)
			*out_action = remappedCommand;
		// return whether a remap was found
		return remapExists;
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
		std::vector<KEYBOARD> vectorKeyboards;
		// to hold each keyboard
		// It would be possible to make a map between keyboard name and keyboard
		// but we won't have that many at once to require such a thing

		

		// Class that will simulate the output for this remapper
		// InputSimulator inputSimulator;

	public:
		
		Remapper() {}


		BOOL ParseSettings(std::wstring filename);

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


	// USAGE: When a remap includes a modifier key (Ctrl, Alt or Shift), sending keystrokes normally
	// would cause the new commands to be affected by the modifier key being held down. For example,
	// mapping Alt+a to send the string "abc" would instead send Alt+a, Alt+b and Alt+c.
	// This function is used to unpress a key, by both setting its thread keyboard state and sending
	// a release INPUT. It does not seem to work everywhere; namely, Google Chrome's URL bar does not
	// recognize characters sent this way. Since behavior is inconsistent, I think it's better to keep
	// both methods of unpressing keys in this function.
	// A few things need to be sorted out still, like the problem with AltGr.
	static BOOL SendKeyUp(WORD vKeyCode)
	{

		// Use setkeyboard state to unpress vKeyCode
		BYTE kbState[256];
		GetKeyboardState((PBYTE)&kbState);

		// In each BYTE of kbState, if the high bit (0x80) is on, key is down
		kbState[vKeyCode] &= ~0x80;				// unset it

		// If vKeyCode is left/right specific, also update the state of the neutral key:
		switch (vKeyCode) {
		case VK_LCONTROL:
		case VK_RCONTROL:
			if ((kbState[VK_LCONTROL] & 0x80) || (kbState[VK_RCONTROL] & 0x80))
				kbState[VK_CONTROL] |= 0x80;
			else
				kbState[VK_CONTROL] &= ~0x80;
			break;
		case VK_LSHIFT:
		case VK_RSHIFT:
			if ((kbState[VK_LSHIFT] & 0x80) || (kbState[VK_RSHIFT] & 0x80))
				kbState[VK_SHIFT] |= 0x80;
			else
				kbState[VK_SHIFT] &= ~0x80;
			break;
		case VK_LMENU:
		case VK_RMENU:
			if ((kbState[VK_LMENU] & 0x80) || (kbState[VK_RMENU] & 0x80))
				kbState[VK_MENU] |= 0x80;
			else
				kbState[VK_MENU] &= ~0x80;
			break;
		}

		INPUT unPress;
		unPress.type = INPUT_KEYBOARD;
		unPress.ki.wVk = vKeyCode;
		unPress.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY;
		unPress.ki.wScan = 0;
		unPress.ki.time = 0;
		SendInput(1, &unPress, sizeof(INPUT));
		
		return TRUE;
		
	}

	
}





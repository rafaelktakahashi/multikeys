// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// SET THIS TO 0 TO DISABLE ALL DEBUG OUTPUT
#define DEBUG				1
#define DEBUG_TEXT_SIZE		128


// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// Additional headers
#include <deque>			// double-ended queues for holding decision records
#include <string>			// std::string and std::wstring
#include <vector>			// contiguous, iterable containers for keyboard structures
#include <unordered_map>	// hash maps for storing the set of remaps for each keyboard
#include <fstream>			// for reading the configuration file
#include <Windows.h>		// for the Windows API
#include <locale>			// setting locale
#include <codecvt>			// for converting strings between different encodings
#include <cctype>			// make sure things like hex digit checking will work (that's also in locale)
#include <shellapi.h>		// to get arguments passed to main


// Next, structs that are referenced globally

// Struct for a simulated shortcut or unicode keypress
struct KEYSTROKE_OUTPUT
{
	BYTE modifiers;		// Extra keys to be pressed with the virtual key. No effect if using unicode
	USHORT vKey;		// Virtual key code to be sent; must be 0 if KEYEVENTF_UNICODE is set
	UINT32 codepoint;	// If KEYEVENTF_UNICODE, send this unicode character instead, ignoring modifiers
	DWORD flags;		// Extra information: we only really use the KEYEVENTF_UNICODE flag. There's KEYUP too.

						// Constructor for a virtual key with modifiers
	KEYSTROKE_OUTPUT(BYTE _modifiers, USHORT _vKey) : modifiers(_modifiers), vKey(_vKey), flags(0)
	{}

	// Constructor for a unicode character (vKey will be 0)
	KEYSTROKE_OUTPUT(UINT32 _codepoint) : codepoint(_codepoint), vKey(0), modifiers(0), flags(KEYEVENTF_UNICODE)
	{}


	KEYSTROKE_OUTPUT() : vKey(0), modifiers(0), flags(0)
	{}
};

// This struct represents a user keystroke (identified by scancode, not virtual key)
// together with modifiers (which have to be virtual keys)
// and RI_KEY flags (keydown, keyup, and e0/e1 prefixes)
// that may or may not be mapped to a KEYSTROKE_OUTPUT
struct KEYSTROKE_INPUT
{
	BYTE modifiers;		// Necessary modifiers to activate this trigger (Ctrl, Alt, Win and Shift, left and right)
	USHORT scancode;	// Physical key that activates this trigger
	USHORT flags;		// 0 - RI_KEY_MAKE; 1 - RI_KEY_BREAK; 2 - RI_KEY_E0; 4 - RI_KEY_E1

	KEYSTROKE_INPUT(BYTE _modifiers, USHORT _scancode, USHORT _flags)
		: modifiers(_modifiers), scancode(_scancode), flags(_flags)
	{}

	KEYSTROKE_INPUT(BYTE _modifiers, USHORT _scancode) : modifiers(_modifiers), scancode(_scancode), flags(0)
	{}

	KEYSTROKE_INPUT(USHORT _scancode, USHORT _flags) : modifiers(0), scancode(_scancode), flags(_flags)
	{}

	KEYSTROKE_INPUT(USHORT _scancode) : modifiers(0), scancode(_scancode), flags(0)
	{}

	KEYSTROKE_INPUT() : modifiers(0), scancode(0xff), flags(0)
	{}		// caution - scancode ff represents keyboard error
};


// Since we map from KEYSTROKE_INPUT to KEYSTROKE_OUTPUT using unordered maps, we need a way to implement a hash
namespace std
{
	template <>
	struct hash<KEYSTROKE_INPUT> : public unary_function<KEYSTROKE_INPUT, size_t>
	{
		size_t operator()(const KEYSTROKE_INPUT& value) const
		{
			return value.scancode % 10;
		}
	};

	template <>
	struct equal_to<KEYSTROKE_INPUT> : public unary_function<KEYSTROKE_INPUT, bool>
	{
		bool operator()(const KEYSTROKE_INPUT& x, const KEYSTROKE_INPUT& y) const
		{
			return (x.scancode == y.scancode && x.flags == y.flags);
		}
	};
}


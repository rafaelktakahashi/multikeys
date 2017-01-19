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
#include <deque>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <Windows.h>
#include <locale>
#include <codecvt>


// Next, structs that are referenced globally

// Struct for a shortcut or unicode keypress
// Aren't we making fools out of ourselves? We can use a RAWKEYBOARD structure instead of this.
struct Keystroke
{
	BYTE modifiers;		// Extra keys to be pressed with the virtual key. No effect if using unicode
	USHORT vKey;		// Virtual key code to be sent; 0 means send unicode char instead
	UINT32 codepoint;	// If virtual key is 0, send this unicode character instead, ignoring modifiers

						// Constructor for a virtual key with modifiers
	Keystroke(BYTE _modifiers, USHORT _vKey) : modifiers(_modifiers), vKey(_vKey)
	{}

	// Constructor for a unicode character (vKey will be 0)
	Keystroke(UINT32 _codepoint) : codepoint(_codepoint), vKey(0), modifiers(0)
	{}


	Keystroke() : vKey(0), modifiers(0)
	{}
};
#pragma once

#include "stdafx.h"


#define MODIFIER_LCTRL		0b10000000	// 128
#define MODIFIER_RCTRL		0b01000000	// 64
#define MODIFIER_LALT		0b00100000	// 32
#define MODIFIER_RALT		0b00010000	// 16
#define MODIFIER_LSHIFT		0b00001000	// 8
#define	MODIFIER_RSHIFT		0b00000100	// 4
#define MODIFIER_LWIN		0b00000010	// 2
#define MODIFIER_RWIN		0x00000001	// 1

namespace Multikeys
{

	// Contains pretend keyboards and "presses" them when prompted.
	class InputSimulator
	{
	private:
		// This is the dummy keyboard that we use to simulate keystrokes
		INPUT simulatedKeyboards[512];
		// For up to 512 inputs in sequence
		// Initialize in constructor

		
		// private UINT SendVirtualKey
		//
		// Sends the virtual key identified by a given virtual key code
		// into the active window. Additional keystrokes are added according
		// to modifiers (ctrl, shift, alt and winkey)
		//
		// Parameters:
		//		BYTE modifiers - flags indicating which (if any) modifier keys
		//				are supposed to be held down when sending the stroke
		//		USHORT vKey - virtual-key code (1 to 254) of the key to be sent
		//		BOOL keyup - flag indicating whether signal is a keyup
		// Return value:
		//		UINT - returns the number of events that were successfully
		//				inserted into the keyboard stream. Failed when 0.
		UINT SendVirtualKey(BYTE modifiers, USHORT vKey, BOOL keyup);


		// private UINT SendUnicodeCharacter
		//
		// Sends the unicode character identified by a given unicode code point
		// into the active window. Characters beyond U+FFFF are sent as two inputs
		// (a UTF-16 surrogate pair)
		//
		// Parameters:
		//		UINT32 codepoint - unicode code point of the character to
		//				simulate; cast to UINT32 to make it clear that this
		//				is a 32-bit value.
		// Return value:
		//		UINT - returns the number of events that were successfully
		//				inserted into the keyboard stream. Failed when 0.
		//
		// Remarks:
		//		An input event sent by this function contains a virtual key value
		//		of 0xe7 (corresponds to no character). It does not generate raw input data.
		UINT SendUnicodeCharacter(UINT32 codepoint, BOOL keyup);


	public:

		// Constructor
		// No inputs, no outputs
		// There should only be one instance of this class at a time
		// Could make a singleton, but what for. This ain't Java.
		InputSimulator();


		// public UINT SendKeyboardInput
		//
		// Sends a keystroke, which may consist either of a unicode code point
		// or a virtual-key code with modifiers.
		//
		// Parameters:
		//		IKeystrokeOutput key - keystroke structure that contains
		//				information about the key to simulate
		// Return value:
		//		UINT - returns the number of events that were successfully
		//				inserted into the keyboard stream. Failed when 0.
		UINT SendKeyboardInput(IKeystrokeOutput * key);	// Maybe we'll support Keystrokes with more options in the future.
													// A good idea is a IKeystrokeOutput with a string in it.
	};
}
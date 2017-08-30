#pragma once
//
#include "stdafx.h"
//#include "Keystrokes.h"
//
//
//namespace Multikeys
//{
//	
//	class Remapper
//	{
//	private:
//		// work variable
//		WCHAR* wcharWork;
//
//
//		// a vector of keyboards
//		std::vector<KEYBOARD> vectorKeyboards;
//		// to hold each keyboard
//		// It would be possible to make a map between keyboard name and keyboard
//		// but we won't have that many at once to require such a thing
//
//	public:
//		
//		Remapper()
//		{
//		}
//
//		BOOL ParseSettings(std::wstring filename);
//
//		// EvaluateKey
//		//
//		// Receives information about a keypress, analyses it, then if there is a remap,
//		//		returns information about the remapped input.
//		//
//		// Parameters:
//		//		RAWKEYBOARD* keypressed - pointer to the structure representing the keyboard input,
//		//				from the Raw Input API.
//		//		WCHAR* deviceName - wide string containing the name of the device that generated the keystroke
//		//		IKeystrokeCommand** out_action - if there is a remap for the keystroke in RAWKEYBOARD* keypressed
//		//				for the keyboard of name WCHAR* deviceName, then this will point to a
//		//				IKeystrokeCommand pointer pointing to the output to be simulated.
//		// Return value:
//		//		TRUE - There is a remap, and it's been placed in *out_action
//		//		FALSE - There is no remap for this key; do not block key
//		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, IKeystrokeCommand** out_action);
//	};
//
//
//
//
//	// USAGE: When a remap includes a modifier key (Ctrl, Alt or Shift), sending keystrokes normally
//	// would cause the new commands to be affected by the modifier key being held down. For example,
//	// mapping Alt+a to send the string "abc" would instead send Alt+a, Alt+b and Alt+c.
//	// This function is used to unpress a key, by both setting its thread keyboard state and sending
//	// a release INPUT. It does not seem to work everywhere; namely, Google Chrome's URL bar does not
//	// recognize characters sent this way. Since behavior is inconsistent, I think it's better to keep
//	// both methods of unpressing keys in this function.
//	// A few things need to be sorted out still, like the problem with AltGr.
//	static BOOL SendKeyUp(WORD vKeyCode);
//
//	
//}
//
//
//
//

#pragma once

#include "stdafx.h"
#include "Level.h"
#include "Modifier.h"

namespace Multikeys
{
	typedef class Keyboard
	{
	private:

		// Set of modifiers registered in this keyboard. This object will listen to
		// these specific keys as modifiers and keep track of their state internally.
		ModifierStateMap * modifierStateMap;

		// Initialize in constructor - keep an empty command in memory, since it's
		// frequently returned.
		IKeystrokeCommand* noAction;

		// All levels belonging to this keyboard's layout
		const std::vector<PLevel> levels;
		// Const containers return const references.

		// Currently active level; null if the current combination of modifiers corresponds
		// to no level.
		Level* activeLevel;

		// Pointer to a dead key waiting for the next character; null if no dead key is active.
		DeadKeyCommand * activeDeadKey;

		// Call this function to check for modifiers.
		// If the key described by the parameters is a modifier, the internal state of
		// this object is updated (as well as the active level), and TRUE is returned.
		// If the key is not a modifier, no changes to internal state are made, and
		// FALSE is returned.
		BOOL _updateKeyboardState(Scancode scancode, BYTE vKeyCode, BOOL flag_keyup);

	public:

		// Public name of this device; wide string in conformity with the Raw Input API.
		std::wstring deviceName;

		// Constructor
		// levelCount - The number of levels this keyboard will contain
		// levels - array of Level pointers. May delete after calling this.
		// modifiers - structure of ModifierStateMap already filled with Modifiers
		//				ownership is transferred to this Keyboard object.
		Keyboard(unsigned int levelCount, const PLevel* levels, ModifierStateMap* modifiers);
		// : levels(levels, levels+levelCount)		<-- use in implementation

		// Receives information about a keypress, and returns true if the keystroke should
		// be blocked.
		// scancode - struct containing the scancode of the keypress to be evaluated
		// vKey - virtual key code for the keypress to be evaluated, contained in a char.
		// flag_keyup - true if this keystroke information is for a key release
		// out_action - pointer to an IKeystrokeCommand*; if this function returns TRUE,
		//			that pointer will point to the remapped command to be executed.
		bool evaluateKey(
			Scancode scancode, BYTE vKey, bool flag_keyup,
			OUT PKeystrokeCommand*const out_action);

		// Set the internal state of all modifiers to unpressed.
		void resetModifierState();

		// Destructor
		~Keyboard();

	} *PKeyboard;
}
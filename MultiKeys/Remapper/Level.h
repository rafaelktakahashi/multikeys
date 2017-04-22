#pragma once

#include "stdafx.h"
#include "Scancode.h"
#include "Modifier.h"
#include "KeystrokeCommands.h"

namespace Multikeys
{

	// This class represents the remaps associated with a specific
	// modifier combination.
	typedef class Level
	{
	private:

		// Map from scancodes to keystroke command pointers.
		// No command pointer is ever deleted at runtime.
		std::map<Scancode, PKeystrokeCommand> layout;

		// Identifies the combination of modifiers that trigger this level.
		ModifierStateMap *const ptrModifierStateMap;
		// Const objects can only call const methods

	public:

		// Constructor
		// modStates - Pointer to a ModifierStateMap already initialized with the
		//			proper modifiers; all modifiers must match that of the
		//			keyboard that owns this level, in order.
		//			The states in modStates will identify this level.
		// remapsCount - the number of remaps registered in this level
		// keys, commands - arrays of keys and command pointers, in the correct order,
		//			representing the remaps. The caller may delete both arrays
		//			afterwards.
		Level(ModifierStateMap* modStates,
			unsigned short remapsCount, Scancode* keys, PKeystrokeCommand* commands);

		// Receives a scancode and returns the command mapped to it.
		// If there is no such command, a null pointer is returned.
		PKeystrokeCommand getCommand(Scancode sc) const;

		// Check if a set of modifiers is equal to this level's.
		bool triggersLevel(ModifierStateMap* modifierState) const;

		

		// Destructor
		~Level();

	} *PLevel;

}
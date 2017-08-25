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
		const std::unordered_map<Scancode, PKeystrokeCommand> layout;

	public:

		// This identifies the combination of modifiers that trigger this level,
		// by name of the modifiers. Each modifier listed must be on,
		// and every other must be off.
		const std::vector<std::wstring> modifierCombination;

		// Updated Constructor
		// modifierCombination - vector of wstrings that contains the names of each
		//		modifier that should be pressed down in order to activate this level.
		//		Modifiers present in the parent keyboard, but not in this list, must
		//		not be pressed in order to activate this level.
		// layout - a hash map from scancode to its command. This object is used purely
		//		for retrieval.
		// The caller may delete either container, or let them go out of scope after calling this.
		Level(const std::vector<std::wstring>& _modifierCombination,
			const std::unordered_map<Scancode, PKeystrokeCommand>& _layout);

		// Receives a scancode and returns the command mapped to it.
		// If there is no such command, a null pointer is returned.
		PKeystrokeCommand getCommand(Scancode sc) const;


		

		// Destructor
		~Level();

	};

}
#pragma once

#include "stdafx.h"
#include "Scancode.h"
#include "KeystrokeCommands.h"

namespace Multikeys
{

	// This class represents the remaps associated with a specific
	// modifier combination.
	class Layer
	{
	private:

		// Map from scancodes to keystroke command pointers.
		// No command pointer is ever deleted at runtime.
		const std::unordered_map<Scancode, BaseKeystrokeCommand*> layout;

	public:

		// This identifies the combination of modifiers that trigger this layer,
		// by name of the modifiers. Each modifier listed must be on,
		// and every other must be off.
		const std::vector<std::wstring> modifierCombination;

		// Updated Constructor
		// modifierCombination - vector of wstrings that contains the names of each
		//		modifier that should be pressed down in order to activate this layer.
		//		Modifiers present in the parent keyboard, but not in this list, must
		//		not be pressed in order to activate this layer.
		// layout - a hash map from scancode to its command. This object is used purely
		//		for retrieval.
		// The caller may delete either container, or let them go out of scope after calling this.
		Layer(const std::vector<std::wstring>& _modifierCombination,
			const std::unordered_map<Scancode, BaseKeystrokeCommand*>& _layout);

		// Receives a scancode and returns the command mapped to it.
		// If there is no such command, a null pointer is returned.
		BaseKeystrokeCommand* getCommand(Scancode sc) const;


		

		// Destructor
		~Layer();

	};

}
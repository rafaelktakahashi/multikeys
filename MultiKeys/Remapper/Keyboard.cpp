#include "stdafx.h"
#include "Keyboard.h"

// Implementation of methods defined in Keyboard.h

namespace Multikeys
{
	Keyboard::Keyboard(const std::vector<Level*>& levels, ModifierStateMap* modifiers)
		: levels(levels), modifierStateMap(modifiers)
	{ }


	bool Keyboard::_updateKeyboardState(Scancode sc, bool flag_keyup)
	{
		return modifierStateMap->updateState(sc, !flag_keyup);
	}


	bool Keyboard::evaluateKey(
		Scancode scancode, BYTE vKey, bool flag_keyup,
		OUT PKeystrokeCommand*const out_action)
	{
		// first implementation

		if (_updateKeyboardState(scancode, flag_keyup))
		{
			// this method returns true if scancode corresponds to a modifier
			// in that case, no action should be taken
			*out_action = noAction;
			return true;
		}

		// Ask the currently active level for the action corresponding to this.
		if (activeLevel == nullptr) return false;
		PKeystrokeCommand command = activeLevel->getCommand(scancode);

		// in case of null (no command corresponds to this scancode), do not block this key
		if (command == nullptr)
			return false;

		// Now, we must check for dead key:
		// 1. If there is an active dead key, the obtained command goes to it
		if (activeDeadKey != nullptr && !flag_keyup)
		{
			if (command != nullptr)
				activeDeadKey->setNextCommand((BaseKeystrokeCommand*)command);
			else
				activeDeadKey->setNextCommand(vKey);

			*out_action = activeDeadKey;
			activeDeadKey = nullptr;
			return true;
		}
		// 2. If obtained command is a dead key, it gets stored in this keyboard
		if (((BaseKeystrokeCommand*)command)->getType() == KeystrokeOutputType::DeadKeyCommand
			&& !flag_keyup)
		{
			activeDeadKey = (DeadKeyCommand*)command;
			*out_action = noAction;
			return true;
		}
		// 3. Return the actual command
		*out_action = command;
		return true;

	}


	void Keyboard::resetModifierState()
	{
		// Lets the modifier state take care of this
		this->modifierStateMap->resetAllModifiers();
		return;
	}

	Keyboard::~Keyboard()
	{
		// Destroy all levels
		for (auto it = this->levels.begin(); it != this->levels.end(); it++)
		{
			delete (*it);
		}
		// Destroy the modifier state map
		delete[] this->modifierStateMap;
	}
}
#include "stdafx.h"
#include "Keyboard.h"
#include "KeystrokeCommands.h"

// Implementation of methods defined in Keyboard.h

namespace Multikeys
{
	Keyboard::Keyboard(const std::wstring name,
		const std::vector<Layer*>& layers, ModifierStateMap* modifiers)
		: layers(layers), modifierStateMap(modifiers), deviceName(name)
	{
		noAction = new EmptyCommand();
		activeDeadKey = nullptr;

		// Initialize the current layer to whichever layer activates with no modifier
		// Note: iterator dereferences into a pointer
		this->activeLayer = nullptr;
		for (auto it = this->layers.begin(); it != this->layers.end(); it++)
		{
			if (this->modifierStateMap->checkState(*it))
			{
				this->activeLayer = *it;
				break;
			}
		}
	}


	bool Keyboard::_updateKeyboardState(Scancode sc, bool flag_keyup)
	{
		// Update the state map
		// If sc is not a modifier, this method will return false:
		if (!modifierStateMap->updateState(sc, !flag_keyup))
			return false;

		// Then, if a modifier changed state, update the currently active layer
		// Note: iterator dereferences into a pointer
		this->activeLayer = nullptr;
		for (auto it = this->layers.begin(); it != this->layers.end(); it++)
		{
			if (this->modifierStateMap->checkState(*it))
			{
				this->activeLayer = *it;
				break;
			}
		}
		return true;
	}


	bool Keyboard::evaluateKey(
		Scancode scancode, BYTE vKey, bool flag_keyup,
		OUT PKeystrokeCommand*const out_action)
	{
		// 1. Correct vKey code (left and right variants)
		// This step is currently skipped because the corrected vkeycodes
		// are not used anywhere else.
		if (0)
		{
			BYTE LRvKey = vKey;
			bool newFlag_E0 = scancode.flgE0;
			if (vKey == VK_SHIFT && scancode == 0x2a)
				LRvKey = VK_LSHIFT;
			else if (vKey == VK_SHIFT && scancode == 0x36)
				LRvKey = VK_RSHIFT;
			else if (vKey == VK_CONTROL)
			{
				LRvKey = (scancode.flgE0 ? VK_RCONTROL : VK_LCONTROL);
				newFlag_E0 = 0;
			}
			else if (vKey == VK_MENU)
			{
				LRvKey = (scancode.flgE0? VK_RMENU : VK_LMENU);
				newFlag_E0 = 0;
			}			// MENU is Alt key

		}

		// 2. Check if received key is a modifier.
		if (_updateKeyboardState(scancode, flag_keyup))
		{
			// If so, return no action but still block the input.
			// No scancode registered as modifier is allowed to also
			// be mapped into something else.
			*out_action = noAction;
			return true;	// Since no action should be taken, input should also be blocked.
		}

		// 3. Ask the currently active layer for the action corresponding to this.
		// If there is no currently active layer (probably because of an invalid
		// combination of modifiers), then the resulting action should be no action.
		BaseKeystrokeCommand* command;
		if (activeLayer == nullptr)
		{
			command = noAction;
		}
		else
		{
			command = activeLayer->getCommand(scancode);
		}


		// 4. In case of a keyup, we should not check for dead keys. That is, return immediately.
		if (flag_keyup)
		{
			*out_action = command;	// <- even if it's null.
			return command;			// true if command is not null
		}

		// 5. If there is an active dead key, the obtained command goes to it,
		// then the dead key (containing the new command) is returned.
		if (activeDeadKey)
		{
			if (command)		// Even if the command is not Unicode.
				activeDeadKey->setNextCommand(command);
			else
				activeDeadKey->setNextCommand(vKey);

			*out_action = activeDeadKey;
			activeDeadKey = nullptr;
			return true;
		}
		// 6. If obtained command is a dead key, it gets stored in this keyboard
		// Try to cast it into a dead key; that will check if the object is a DeadKeyCommand,
		// and will also result in the already cast pointer.
		DeadKeyCommand* deadKeyCommand = dynamic_cast<DeadKeyCommand*>(command);
		if (deadKeyCommand)
		{
			activeDeadKey = deadKeyCommand;
			*out_action = noAction;
			return true;
		}

		// 7. Return the actual command
		*out_action = command;	// even if it's null
		return command;			// returns true if non-null

	}


	void Keyboard::resetModifierState()
	{
		// Lets the modifier state take care of this
		this->modifierStateMap->resetAllModifiers();
		// But also update the current layer
		for (auto it = this->layers.begin(); it != this->layers.end(); it++)
		{
			if (this->modifierStateMap->checkState(*it))
			{
				this->activeLayer = *it;
				break;
			}
			this->activeLayer = nullptr;
		}
		return;
	}

	Keyboard::~Keyboard()
	{
		// Destroy all layers
		for (auto it = this->layers.begin(); it != this->layers.end(); it++)
		{
			delete (*it);
		}
		// Destroy the modifier state map
		delete this->modifierStateMap;
	}
}
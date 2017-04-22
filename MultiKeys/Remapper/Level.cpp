#include "stdafx.h"
#include "Level.h"

// Implementation of methods defined in Level.h

namespace Multikeys
{
	Level::Level(ModifierStateMap* modStates,
		unsigned short remapsCount, Scancode* keys, PKeystrokeCommand* commands)
		: ptrModifierStateMap(modStates)
	{
		for (unsigned short i = 0; i < remapsCount; i++)
		{
			this->layout.insert(
				std::pair<Scancode, PKeystrokeCommand>(keys[i], commands[i])
			);
		}
	}

	PKeystrokeCommand Level::getCommand(Scancode sc) const
	{

		if (layout.count(sc) == 0)
		{
			// not found
			return nullptr;
		}
		else
		{
			// found
			return layout.at(sc);
		}
	}

	bool Level::triggersLevel(ModifierStateMap* modifierState) const
	{
		return ( *modifierState == *(this->ptrModifierStateMap) );
	}

	Level::~Level()
	{
		delete this->ptrModifierStateMap;

		for (auto it = this->layout.begin(); it != this->layout.end(); it++)
		{
			delete it->second;
		}
	}
}
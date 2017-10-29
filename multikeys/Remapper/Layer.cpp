#include "stdafx.h"
#include "Layer.h"

// Implementation of methods defined in Layer.h

namespace Multikeys
{
	Layer::Layer(const std::vector<std::wstring>& _modifierCombination,
		const std::unordered_map<Scancode, BaseKeystrokeCommand*>& _layout)
		:	modifierCombination(_modifierCombination),
			layout(_layout)
	{ }

	BaseKeystrokeCommand* Layer::getCommand(Scancode sc) const
	{
		// Because maps do not allow duplicate keys,
		// unordered_map::count actually returns either 0 or 1
		// and is used to tell whether a key exists in the map.
		return (
			layout.count(sc) == 0 ?		// <- true if not found
			nullptr :
			layout.at(sc)
			);
	}

	Layer::~Layer()
	{
		// Delete every command.
		// No command appears in more than one layer, or in more than one keyboard.
		for (auto it = this->layout.begin(); it != this->layout.end(); it++)
		{
			delete it->second;
		}
	}
}
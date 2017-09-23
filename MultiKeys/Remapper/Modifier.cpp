#include "stdafx.h"

// Implementation of methods in Modifier.h
#include "Modifier.h"

namespace Multikeys
{

	/*
	BaseModifier
	*/

	BaseModifier::BaseModifier(std::wstring name) : name(name)
	{ }

	// Pure virtual destructors need an implementation
	BaseModifier::~BaseModifier() { }



	/*
	SimpleModifier
	*/

	SimpleModifier::SimpleModifier(std::wstring name, Scancode scancode) :
		BaseModifier(name), sc(scancode)
	{}

	bool SimpleModifier::matches(Scancode sc) const
	{
		return this->sc == sc;
	}

	SimpleModifier::~SimpleModifier() { }



	/*
	CompositeModifier
	*/

	CompositeModifier::
		CompositeModifier(std::wstring name, std::vector<Scancode> scancodeVector) :
		BaseModifier(name), scanCount(scancodeVector.size())
	{
		scanArray = new Scancode[scancodeVector.size()];
		for (size_t i = 0; i < scancodeVector.size(); i++)
		{
			scanArray[i] = scancodeVector[i];
		}
	}

	bool CompositeModifier::matches(Scancode sc) const
	{
		// An incoming scancode only needs to match one of this modifier's scancodes.
		for (size_t i = 0; i < scanCount; i++)
		{
			if (scanArray[i] == sc)
				return true;
		}
		return false;
	}

	CompositeModifier::~CompositeModifier()
	{
		delete[] scanArray;
	}




	/*
	ModifierStateMap
	*/

	ModifierStateMap::ModifierStateMap(const ModifierStateMap& original)
	{
		// copy constructor also sets all modifiers to false
		this->modifiers =
			original.modifiers;
		for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
		{
			it->second = false;
		}
	}

	ModifierStateMap::ModifierStateMap(const std::vector<PModifier>& modifiers)
	{
		for (size_t i = 0; i < modifiers.size(); i++)
		{
			this->modifiers.insert(std::pair<PModifier, bool>(modifiers[i], false));
		}
	}

	bool ModifierStateMap::updateState(Scancode sc, bool keyDown)
	{
		for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
		{
			if (it->first->matches(sc))
			{
				it->second = keyDown;
				return true;
			}
		}
		return false;
	}

	bool ModifierStateMap::checkState(Layer* const layer) const
	{
		// 0. Layer's modNames contains precisely the modifiers that trigger it.
		// 1. Every name present in modNames must be already set to true in this->modifiers
		// 2. Every name not present in modNames must be false in this->modifiers
		// Satified those conditions, this method must return true
		// Not satified those conditions, this method must return false.
		size_t i = 0;
		bool found = false;

		// Loop through all modifiers in this map
		for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
		{
			// Determine whether or not this modifier is present in the Layer's modifiers
			// Look for it by name
			for (i = 0, found = false; i < layer->modifierCombination.size(); i++)
			{
				if (layer->modifierCombination[i] == it->first->name)	// comparing wstrings
				{
					found = true;
					break;	// get out of this inner loop
				}
			}
			// Presence in the Layer's modifers must be equal to the modifier's current state
			// That is, if the modifier exists in the Layer, then it must be currently pressed
			// if the modifier was not found in the Layer, then it must be currently unpressed.
			if (found != it->second)
			{
				return false;
			}
		}
		// Looped and never returned false, then succeeded.
		return true;
	}

	void ModifierStateMap::resetAllModifiers()
	{
		for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
			it->second = false;
	}

	// This is currently not in use, I believe.
	inline bool ModifierStateMap::operator==(const ModifierStateMap& rhs) const
	{
		return rhs.modifiers.size() == this->modifiers.size()
			&& (		// This is comparing pointers
				std::equal(this->modifiers.begin(), this->modifiers.end(),
					rhs.modifiers.begin())
				);
	}

	ModifierStateMap::~ModifierStateMap()
	{
		// free all PModifiers
		for (auto it = modifiers.begin(); it != modifiers.end(); it++)
		{
			delete it->first;
		}
	}


}
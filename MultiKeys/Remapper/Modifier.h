#pragma once

#include "stdafx.h"
#include "Scancode.h"

namespace Multikeys
{
	// Abstract class
	typedef class AbstractModifier
	{
	protected:
		AbstractModifier(std::wstring name) : name(name)
		{}
	public:

		// This name should uniquely identify each modifier.
		std::wstring name;

		// Check if a given scancode triggers this modifier
		virtual bool matches(Scancode sc) const = 0;

		// Ensure that derived constructors are called
		virtual ~AbstractModifier() = 0;
	} *PModifier;


	// Modifiers identified by a single scancode
	class SimpleModifier : public AbstractModifier
	{
	protected:
		Scancode sc;
	public:
		SimpleModifier(std::wstring name, Scancode scancode) :
			AbstractModifier(name), sc(scancode)
		{}

		bool matches(Scancode sc) const override
		{
			return this->sc == sc;
		}

		~SimpleModifier() override
		{}

	};


	// Modifiers identified by multiple scancodes (ex. Shift)
	class CompositeModifier : public AbstractModifier
	{
	protected:
		Scancode* scanArray;
		unsigned short scanCount;
	public:
		
		// Classic C constructor
		// name - name of this modifier, should be unique across different modifiers.
		// scancodeCount - Number of different scancodes that are part of this composite modifier.
		// scancodeArray - c-style array that contains all Scancodes that are part of this modifier.
		//				the caller may free scancodeArray after this call.
		CompositeModifier(std::wstring name, unsigned short scancodeCount, Scancode* scancodeArray) :
			AbstractModifier(name), scanCount(scancodeCount)
		{
			scanArray = new Scancode[scancodeCount];
			for (size_t i = 0; i < scancodeCount; i++)
			{
				scanArray[i] = scancodeArray[i];
			}
		}

		// STL constructor
		// The caller may free scancodeVector afterwards or let it go out of scope.
		CompositeModifier(std::wstring name, std::vector<Scancode> scancodeVector) :
			AbstractModifier(name), scanCount(scancodeVector.size())
		{
			std::copy(scancodeVector.begin(), scancodeVector.end(), &scanArray[0]);
		}

		bool matches(Scancode sc) const override
		{
			// An incoming scancode only needs to match one of this modifier's scancodes.
			for (unsigned short i = 0; i < scanCount; i++)
			{
				if (scanArray[i] == sc)
					return true;
			}
			return false;
		}

		~CompositeModifier() override
		{
			delete[] scanArray;
		}
	};


	// This class is used for two purposes:
	//		In a Keyboard to represent the internal state of its modifiers (mutable)
	//		In a Level to represent the required modifiers to trigger it (const)
	class ModifierStateMap
	{
	private:

		// ordered map allows for easy iteration
		// The boolean value of each modifier is true if the key is currently pressed.
		std::map<PModifier, bool> modifiers;

		

	public:

		// Copy constructor
		ModifierStateMap(const ModifierStateMap& original)
		{
			// copy constructor also sets all modifiers to false
			this->modifiers =
				original.modifiers;
			for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
			{
				it->second = false;
			}
		}

		// STL constructor; sets all modifiers to unpressed
		ModifierStateMap(const std::vector<PModifier>& modifiers)
		{
			for (size_t i = 0; i < modifiers.size(); i++)
			{ this->modifiers.insert(std::pair<PModifier, bool>(modifiers[i], false)); }
		}
		

		// Receives a scancode, and a flag for keypress up or down.
		// If sc is a modifier contained in this object, its state is updated and true is returned
		// otherwise, false is returned.
		bool updateState(Scancode sc, bool keyDown)
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

		// Do we really need this function?
		bool setState(std::wstring name, bool state)
		{
			// loop through this map; if a modifier with this name is found, update its state
			for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
			{
				if (it->first->name.compare(name) == 0)
				{
					it->second = state;
					return true;
				}
			}
			return false;
		}

		// Returns true if level is triggered by the current combination of modifiers in this object.
		bool checkState(Level* const level) const
		{
			// 1. Every name present in modNames must be set to true in this->modifiers
			// 2. Every name not present in modNames must be set to false in this->modifiers
			// Satified those conditions, this method must return true
			// Not satified those conditions, this method must return false.
			size_t i = 0;
			bool found = false;

			// Loop through list of all modifiers
			for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
			{
				// Determine whether or not this modifier is present in the Level's modifiers
				// Look for it by name
				for (i = 0, found = false; i < level->modifierCombination.size(); i++)
				{
					if (level->modifierCombination[i] == it->first->name)
					{
						found = true;
						break;	// get out of for loop
					}
				}
				// Presence in the Level's modifers must be equal to the modifier's current state
				if (found != it->second)
				{
					return false;
				}
			}
			// Looped and never returned false, then succeeded.
			return true;
		}

		void resetAllModifiers()
		{
			for (auto it = this->modifiers.begin(); it != this->modifiers.end(); it++)
				it->second = false;
		}


		inline bool operator==(const ModifierStateMap& rhs) const
		{
			return rhs.modifiers.size() == this->modifiers.size()
				&& (		// This is comparing pointers
					std::equal(this->modifiers.begin(), this->modifiers.end(),
						rhs.modifiers.begin())
					);
		}

		~ModifierStateMap()
		{
			// free all PModifiers
			for (auto it = modifiers.begin(); it != modifiers.end(); it++)
			{
				delete it->first;
			}
		}
	};
}
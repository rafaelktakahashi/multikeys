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

		// not strictly necessary, but convenient to have (for the remapper)
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
	protected:

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

		// All modifiers are set to not pressed down
		ModifierStateMap(unsigned short modifierCount, PModifier* modifierArray)
		{
			for (unsigned short i = 0; i < modifierCount; i++)
			{
				modifiers.insert(std::pair<PModifier, bool>(modifierArray[i], false));
			}
		}
		// STL constructor; sets all modifiers to unpressed
		ModifierStateMap(std::vector<PModifier> modifiers)
			: ModifierStateMap(modifiers.size(), modifiers.data()) { }

		
		// Constructor with states
		ModifierStateMap(unsigned short modifierCount, PModifier* modifierArray, bool* states)
		{
			// Could use an initializer list, but I don't know if that works for a map
			for (unsigned short i = 0; i < modifierCount; i++)
			{
				modifiers.insert(std::pair<PModifier, bool>(modifierArray[i], states[i]));
			}
		}
		// STL constructor with states (vector<bool> is not really STL and can't be used)
		ModifierStateMap(std::vector<PModifier> modifiers, bool* states)
			: ModifierStateMap(modifiers.size(), modifiers.data(), states) { }

		// This method should not be used by a level (not const)
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

		// (not const)
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
#pragma once

#include "stdafx.h"
#include "Scancode.h"
#include "Layer.h"

namespace Multikeys
{
	// Abstract class
	//
	// Splitting modifiers into either simple or composite
	// is particular to this implementation.
	typedef class BaseModifier
	{
	protected:
		BaseModifier(std::wstring name);
	public:

		// This name should uniquely identify each modifier.
		std::wstring name;

		// Check if a given scancode triggers this modifier
		virtual bool matches(Scancode sc) const = 0;

		// Ensure that derived constructors are called
		virtual ~BaseModifier() = 0;
	} *PModifier;


	// Modifiers identified by a single scancode
	class SimpleModifier : public BaseModifier
	{
	protected:
		Scancode sc;
	public:
		SimpleModifier(std::wstring name, Scancode scancode);

		bool matches(Scancode sc) const override;

		~SimpleModifier() override;

	};


	// Modifiers identified by multiple scancodes (ex. Shift)
	class CompositeModifier : public BaseModifier
	{
	protected:
		Scancode* scanArray;
		size_t scanCount;
	public:
		
		// The caller may free scancodeVector afterwards or let it go out of scope.
		CompositeModifier(std::wstring name, std::vector<Scancode> scancodeVector);

		bool matches(Scancode sc) const override;

		~CompositeModifier() override;
	};


	// This class is used for two purposes:
	//		In a Keyboard to represent the internal state of its modifiers (mutable)
	//		In a Layer to represent the required modifiers to trigger it (const)
	class ModifierStateMap
	{
	private:

		// ordered map allows for easy iteration
		// The boolean value of each modifier is true if the key is currently pressed.
		std::map<PModifier, bool> modifiers;

		

	public:

		// Copy constructor
		ModifierStateMap(const ModifierStateMap& original);

		// STL constructor; sets all modifiers to unpressed
		ModifierStateMap(const std::vector<PModifier>& modifiers);
		

		// Receives a scancode, and a flag for keypress up or down.
		// If sc is a modifier contained in this object, its state is updated and true is returned
		// otherwise, false is returned.
		bool updateState(Scancode sc, bool keyDown);

		// Returns true if layer is triggered by the current combination of modifiers in this object.
		bool checkState(Layer* const layer) const;

		void resetAllModifiers();


		inline bool operator==(const ModifierStateMap& rhs) const;

		~ModifierStateMap();
	};
}
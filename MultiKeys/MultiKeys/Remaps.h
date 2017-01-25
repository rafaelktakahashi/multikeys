#pragma once

#include "stdafx.h"

#include "InputSimulator.h"


// Only the remapper and the parser need to know about this.
// Used to internally store a group of remaps associated to a specific keyboard
struct KEYBOARD
{
	// Name of this device
	WCHAR * device_name;

	// Size of buffer
	USHORT const device_name_sizeof = 128;

	// The simulator that sends keystrokes

	// Map between inputs and outputs
	std::unordered_map<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT> remaps;

	KEYBOARD()
	{
		device_name = new WCHAR[device_name_sizeof];
	}

	// Refresh: Will use another space for device name, and clear the map
	void Clear()
	{
		device_name = new WCHAR[device_name_sizeof];
		remaps.clear();
	}
};


namespace Multikeys
{
	
	class Remapper
	{
	private:
		// work variable
		WCHAR* wcharWork;

		// a vector of keyboards
		std::vector<KEYBOARD> keyboards;
		// to hold each keyboard
		// It would be possible to make a map between keyboard name and keyboard
		// but we won't have that many at once to require such a thing

		

		// Class that will simulate the output for this remapper
		InputSimulator inputSimulator;

	public:
		
		// Constructor
		// Parameters:
		//		std::string - full path to the configuration file to be read
		Remapper(std::string filename);
		Remapper(std::wstring filename);
		// Constructor
		//
		// Creates an instance with an empty map. Must be properly initialized with LoadSettings
		//
		// Parameters: none
		Remapper();

		// Loads the settings in file at filename into memory
		BOOL LoadSettings(std::string filename);
		BOOL LoadSettings(std::wstring filename);

		// EvaluateKey
		//
		// Receives information about a keypress, analyses it, then if there is a remap,
		//		returns information about the remapped input.
		//
		// Parameters:
		//		RAWKEYBOARD* keypressed - pointer to the structure representing the keyboard input,
		//				from the Raw Input API.
		//		WCHAR* deviceName - wide string containing the name of the device that generated the keystroke
		//		KEYSTROKE_OUTPUT* out_action - if there is a remap for the keystroke in RAWKEYBOARD* keypressed
		//				for the keyboard of name WCHAR* deviceName, then this will point to a
		//				KEYSTROKE_OUTPUT containing the keystroke or unicode code point to be simulated.
		// Return value:
		//		TRUE - There is a remap, and it's been placed in *out_action
		//		FALSE - There is no remap for this key.
		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, KEYSTROKE_OUTPUT* out_action);


		BOOL SimulateKeystroke(KEYSTROKE_OUTPUT key);



	};






	// LoadSettings will use this class (only one of its methods, actually
	// (making a static class instead of a namespace so that a bunch of its
	// methods can be private and out of the way)
	class Parser
	{
	private:
		Parser() {}	// Nobody can make an instance of this.


					// Reads alphabetic characters until a non-alphanumeric character is found
					// The non-alpha is not read and not extracted
					// Result is stored in the symbol parameter
					// return value of FALSE means no symbol was found (end-of-file was found instead)
		static BOOL ReadSymbol(std::ifstream* stream, std::string* symbol);
		



		// Read a keyboard name, such as \\?\ACPI#PNP0303#4&231b8599&0#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}
		// and store it in kb's device_name field
		// Will only stop at whitespace (including newline) or a right parenthesis
		static BOOL ReadKeyboardName(std::ifstream* stream, KEYBOARD* kb);


		static BOOL ReadModifierHex(std::ifstream* stream, BYTE* modifiers, UINT32* code);
		


		// Read an input keystroke, such as <+<!0x47
		// (modifiers and a scancode)
		// and place it into a trigger structure
		// Returns FALSE upon encountering an unrecognized character
		static BOOL ReadInputKeystroke(std::ifstream* stream, KEYSTROKE_INPUT* trigger);



		// Read a virtual key, together with modifiers, such as <^0x41
		static BOOL ReadVirtualkey(std::ifstream* stream, KEYSTROKE_OUTPUT* keystroke);


		// Expects a hexadecimal value
		static BOOL ReadUnicode(std::ifstream* stream, KEYSTROKE_OUTPUT* keystroke);


	public:

		// This isn't an interpreter.
		static BOOL ReadFile(std::ifstream* stream, std::vector<KEYBOARD> * ptrVectorKeyboard);
	};

	
}





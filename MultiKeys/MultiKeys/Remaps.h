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
	std::unordered_map<KEYSTROKE_INPUT, IKeystrokeOutput*> remaps;

	KEYBOARD()
	{
		device_name = new WCHAR[device_name_sizeof];
	}

	/*				Need to sort this out later
	~KEYBOARD()
	{
		delete[] device_name;
		for (auto iterator = remaps.begin(); iterator != remaps.end(); iterator++)
		{
			delete[] iterator->second->keystrokesUp;
			delete[] iterator->second->keystrokesDown;
			delete[] iterator->second;
		}
	}
	*/

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
		//		IKeystrokeOutput* out_action - if there is a remap for the keystroke in RAWKEYBOARD* keypressed
		//				for the keyboard of name WCHAR* deviceName, then this will point to a
		//				IKeystrokeOutput containing the keystroke or unicode code point to be simulated.
		// Return value:
		//		TRUE - There is a remap, and it's been placed in *out_action
		//		FALSE - There is no remap for this key.
		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, IKeystrokeOutput** out_action);


		BOOL SimulateKeystroke(IKeystrokeOutput * key);



	};






	

	
}





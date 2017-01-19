#pragma once

#include "stdafx.h"

#include "InputSimulator.h"

namespace Multikeys
{
	class Remapper
	{

		// Nested definition, because no one else needs to know about this.
		struct KEYBOARD
		{
			// Name of this device
			WCHAR * device_name;

			// Size of buffer
			USHORT const device_name_sizeof = 128;

			// The simulator that sends keystrokes

			// Map between scancodes and their unicode remaps
			// 32-bits can represent any unicode code point
			std::unordered_map<USHORT, Keystroke> remaps;

			KEYBOARD()
			{
				device_name = new WCHAR[device_name_sizeof];
			}
		};



		// work variable
		WCHAR* wcharWork;

		// a vector of keyboards
		std::vector<KEYBOARD> keyboards;
		// to hold each keyboard



		// Loads the settings in file at filename into memory
		BOOL LoadSettings(std::string filename);



		// Class that will simulate the output for this remapper
		InputSimulator inputSimulator;

	public:
		
		
		// Constructor
		// Parameters:
		//		std::string - full path to the configuration file to be read
		Remapper(std::string filename);

		// EvaluateKey
		//
		// Receives information about a keypress, analyses it, then if there is a remap,
		//		returns information about the remapped input.
		//
		// Parameters:																				// OUTDATED DESCRIPTION
		//		RAWKEYBOARD* - pointer to structure containing raw input information
		//				about keypress
		//		WCHAR* - string of wchar_t containing name of the device that sent the signal
		// Return:
		//		Keystroke - struct representing what should be sent instead of
		//				the intercepted key.
		//		NULL - there is no remap for this keypress.
		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, Keystroke * out_action);


		BOOL SimulateKeystroke(Keystroke key);

		// ReloadSettings
		//
		// Replace all remaps in memory with the settings in file at filename.
		//
		// Parameters:
		//		std::string - full path to the configuration file to be read
		// Return:
		//		TRUE - Reload succeeded
		//		FALSE - Reload was unsuccessful
		BOOL ReloadSettings(std::string filename);
		
	};
}

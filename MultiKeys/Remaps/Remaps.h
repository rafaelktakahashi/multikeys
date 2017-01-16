#pragma once

#include <Windows.h>
#include "stdafx.h"


namespace Remaps				// We'll need this to be a class in order to hold the remaps (probably in a map)
								// Would be good to have a few structs, too
{

	// BOOL EvaluateKey
	//
	//	Evaluates information about a keypress.
	//
	// Parameters:
	//		RAWKEYBOARD * keypressed - pointer to the RAWKEYBOARD structure that contains data about the key press
	//		WCHAR * deviceName - unicode string (wchar_t) containing the device name
	// Return value:
	//		TRUE - Key should be blocked
	//		FALSE - Key should not be blocked
	BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName);

	// BOOL LoadSettings
	//
	//	Read remaps from a file into memory.
	//
	// Parameters:
	//		None yet.
	// Return value:
	//		TRUE - Remaps have been successfully loaded into memory
	//		FALSE - Remaps were unsuccessful.
	BOOL LoadSettings();

	


	
}


namespace Multikeys
{
	class Remapper
	{
	public:
		Remapper(std::string filename);
		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName);
		BOOL ReloadSettings(std::string filename);
	};
}


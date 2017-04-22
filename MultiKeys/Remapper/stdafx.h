// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// C Runtime header files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Additional headers
#include <Windows.h>			// for the Windows API
#include <string>				// std::string and std::wstring
#include <vector>				// contiguous, iterable containers for keyboard structures
#include <array>				// contiguous, fixed-length containers for modifiers
#include <map>					// maps for dead keys
#include <unordered_map>		// hash maps for storing the set of remaps for each keyboard
#include <fstream>				// for reading the configuration file
#include <locale>				// for setting locale if needed
#include <codecvt>				// for converting strings between different encondings
#include <cctype>				// make sure things like hex digit checking will work (that's also in locale)
#include <shellapi.h>			// to get arguments passed to main

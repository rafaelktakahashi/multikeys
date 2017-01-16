// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers



// Additional headers
#include <Windows.h>
#include <vector>		// for each keyboard
#include <map>			// to hold the remaps
#include <list>			// used in implementation of a remap
#include <fstream>		// read the configuration file containing the remaps
#include <string>		// to use std::strings
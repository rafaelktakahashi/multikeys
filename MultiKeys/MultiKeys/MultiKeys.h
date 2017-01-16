// Based on code by Vít Blecha, developed by Rafael Takahashi

// MultiKeys allows you to selectively remap keys on multiple keyboards, with
// an intuitive interface and no need to learn a scripting language.

#pragma once

#include "resource.h"

// SET THIS TO 0 TO DISABLE ALL DEBUG OUTPUT
#define DEBUG				1
#define DEBUG_TEXT_SIZE		128

// Structure of a single record that will be saved in the decisionBuffer
struct DecisionRecord
{
	RAWKEYBOARD keyboardInput;
	BOOL decision;

	DecisionRecord(RAWKEYBOARD _keyboardInput, BOOL _decision) : keyboardInput(_keyboardInput), decision(_decision)
	{
		// Constructor
	}
};



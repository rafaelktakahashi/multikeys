// Based on code by Vít Blecha, developed by Rafael Takahashi

// MultiKeys allows you to selectively remap keys on multiple keyboards, with
// an intuitive interface and no need to learn a scripting language.

#pragma once

#include "resource.h"



// Structure of a single record that will be saved in the decisionBuffer
struct DecisionRecord
{
	// Information about the keypress that generated this record
	RAWKEYBOARD keyboardInput;

	// Information about the action to be taken, if any
	KEYSTROKE_OUTPUT mappedInput;

	// TRUE - this keypress should be blocked, and mappedInput should be carried out
	// FALSE - this keypress should not be blocked, and there is no mapped input to be carried out
	BOOL decision;

	DecisionRecord(RAWKEYBOARD _keyboardInput, BOOL _decision) : keyboardInput(_keyboardInput), decision(_decision)
	{
		// Constructor
	}

	DecisionRecord(RAWKEYBOARD _keyboardInput, KEYSTROKE_OUTPUT _mappedInput, BOOL _decision)
		: keyboardInput(_keyboardInput), mappedInput(_mappedInput), decision(_decision)
	{
		// Constructor
	}
};



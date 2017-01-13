// Based on code by Vít Blecha, developed by Rafael Takahashi

// MultiKeys allows you to selectively remap keys on multiple keyboards, with
// an intuitive interface and no need to learn a scripting language.

#pragma once

#include "resource.h"

// Structure of a single record that will be saved in the decisionBuffer
struct DecisionRecord
{
	USHORT virtualKeyCode;
	BOOL decision;

	DecisionRecord(USHORT _virtualKeyCode, BOOL _decision) : virtualKeyCode(_virtualKeyCode), decision(_decision)
	{
		// Constructor
	}
};
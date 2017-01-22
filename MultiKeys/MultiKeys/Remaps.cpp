#include "stdafx.h"

#include "Remaps.h"



Multikeys::Remapper::Remapper(std::string filename)					// constructor
{
	// Setup a keyboard input simulator
	inputSimulator = InputSimulator();

	if (!LoadSettings(filename))
	{
		
	}
	OutputDebugString(L"Initialized.");
}
Multikeys::Remapper::Remapper(std::wstring filename)
{
	// Setup a keyboard input simulator
	inputSimulator = InputSimulator();

	if (!LoadSettings(filename))
	{ }
	OutputDebugString(L"Initialized.");
}
Multikeys::Remapper::Remapper()
{
	// InputSimulator is not initialized;
}




BOOL Multikeys::Remapper::LoadSettings(std::string filename)		// parser
{
	return Parser::ReadFile(filename, &keyboards);
}
BOOL Multikeys::Remapper::LoadSettings(std::wstring filename)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return Parser::ReadFile(converter.to_bytes(filename), &keyboards);
}


// Temporary function to test the parser
void ParserTest()
{
	return;
}






BOOL Multikeys::Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, KEYSTROKE_OUTPUT * out_action)
{
	// Make an input

	BYTE modifiers = 0;
	// GetKeyState checks the state of a virtual key at the time of this event, not this very moment
	// Most significant bit is 1 if key is down
	modifiers += (GetKeyState(VK_LCONTROL & 0x8000) ? MODIFIER_LCTRL : 0);
	modifiers += (GetKeyState(VK_RCONTROL & 0x8000) ? MODIFIER_RCTRL : 0);
	modifiers += (GetKeyState(VK_LSHIFT & 0x8000) ? MODIFIER_LSHIFT : 0);
	modifiers += (GetKeyState(VK_RSHIFT & 0x8000) ? MODIFIER_RSHIFT : 0);
	modifiers += (GetKeyState(VK_LMENU & 0x8000) ? MODIFIER_LALT : 0);
	modifiers += (GetKeyState(VK_RMENU & 0x8000) ? MODIFIER_RALT : 0);
	modifiers += (GetKeyState(VK_LWIN & 0x8000) ? MODIFIER_LWIN : 0);
	modifiers += (GetKeyState(VK_RWIN & 0x8000) ? MODIFIER_RWIN : 0);
	KEYSTROKE_INPUT input = KEYSTROKE_INPUT(modifiers, keypressed->MakeCode, keypressed->Flags);
	// We'll look for a similar one in our list
	
	// Look for correct device; return FALSE (= do not block) otherwise
	for (auto iterator = keyboards.begin(); iterator != keyboards.end(); iterator++)
	{
		if (wcscmp(iterator->device_name, deviceName) == 0)
		{
			// found it!
			// check if the remaps map for this device contains our scancode
			auto innerIt = iterator->remaps.find(input);
			if (innerIt != iterator->remaps.end())
			{
				// Reaching this point means that the codepoint was found. Return it
				if (keypressed->Flags & RI_KEY_BREAK ? 0 : 1)
				{
					// Copy the keystroke
					*out_action = innerIt->second;		// "return" as in the out parameter
					// We don't want to copy a reference. That would lead to the map itself.
					return TRUE;
				}
				// This is a keyup.
				*out_action = innerIt->second;
				out_action->flags |= KEYEVENTF_KEYUP;
				return TRUE;
			}
			else
				return FALSE;
			
		}
	}

	return FALSE;
}


// oh, look, wrappers
BOOL Multikeys::Remapper::SimulateKeystroke(KEYSTROKE_OUTPUT key)
{
	return inputSimulator.SendKeyboardInput(key);
}







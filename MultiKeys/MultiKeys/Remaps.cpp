#include "stdafx.h"

#include "Remaps.h"



Multikeys::Remapper::Remapper(std::string filename)					// constructor
{
	// Setup a keyboard input simulator
	inputSimulator = InputSimulator();

	if (!LoadSettings(filename))
	{
		// Error; figure out what to do.
	}
}



// A static class dedicated to parsing the configuration file is under development
// When that's ready, this function should just call that.
BOOL Multikeys::Remapper::LoadSettings(std::string filename)		// parser
{
	setlocale(LC_ALL, "");
	// in-file-stream: will only read
	std::ifstream file(filename.c_str());

	// hold each line here:
	std::string line;

	// from <codecvt>, the object that will convert between narrow and wide strings (C++11 and newer)
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

	if (!file.is_open())
		return FALSE;	// oh, no!

						// read line by line:
	while (getline(file, line))		// guarantees read until newline, unlike file >> line
	{
		// device_begin
		// read device name into new KEYBOARD structure
		// add its remaps one by one
		if (line == "device_begin")
		{
			// next line is the keyboard name
			std::getline(file, line);
			KEYBOARD thisKeyboard = KEYBOARD();
			// copy keyboard name into newly created keyboard
			// (swprintf_s because destination is in utf-16
			auto wideString = std::wstring();

			wideString = converter.from_bytes(line);							// %ls is wide string
			swprintf_s(thisKeyboard.device_name, thisKeyboard.device_name_sizeof, L"%ls", wideString.c_str());
			// read the remaps one by one
			while (getline(file, line))
			{
				if (line == "remap")
				{
					// parse next line into hexadecimal values
					getline(file, line);
					USHORT scancode;
					UINT codePoint;

					// sscanf_s: from string into anything - should return the amount of scans
					if (sscanf_s(line.c_str(), "%hx %x", &scancode, &codePoint) != 2)
						return FALSE;			// hx is short-sized hex, x is int-sized hex

					thisKeyboard.remaps.insert(std::pair<USHORT, KEYSTROKE_OUTPUT>(scancode, KEYSTROKE_OUTPUT(codePoint)));
				}
				else if (line == "device_end")
				{
					break;	// break the loop and expect a possible next device
				}
				else
				{
					// error: unexpected term
					return FALSE;
				}
			}
			// finished this device. We'll add it to the list of devices
			keyboards.push_back(thisKeyboard);
		}
		else
		{
			// error: unexpected term
			return FALSE;
		}

	}
	return TRUE;
}


// Temporary function to test the parser
void ParserTest()
{
	auto tester = std::unordered_map<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT>();
	BOOL res = Parser::ReadFile(std::string("C:\\MultiKeys\\Multi"), &tester);
	return;
}






BOOL Multikeys::Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, KEYSTROKE_OUTPUT * out_action)
{
	// Get scancode (physical key)
	USHORT scancode = keypressed->MakeCode;
	
	// Look for correct device; return FALSE (= do not block) otherwise
	for (auto iterator = keyboards.begin(); iterator != keyboards.end(); iterator++)
	{
		if (wcscmp(iterator->device_name, deviceName) == 0)
		{
			// found it!
			// check if the remaps map for this device contains our scancode
			auto innerIt = iterator->remaps.find(scancode);
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


// oh, look, a wrapper
BOOL Multikeys::Remapper::SimulateKeystroke(KEYSTROKE_OUTPUT key)
{
	return inputSimulator.SendKeyboardInput(key);
}


BOOL Multikeys::Remapper::ReloadSettings(std::string filename)
{
	LoadSettings(filename);
	return TRUE;
}





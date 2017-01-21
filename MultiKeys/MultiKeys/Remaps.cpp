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
	;
}

// LoadSettings will use this class (only one of its methods, actually
// (making a static class instead of a namespace so that a bunch of its
// methods can be private and out of the way)
class Parser
{
private:
	Parser() {}	// Nobody can make an instance of this.


	// Reads alphabetic characters until a non-alphanumeric character is found
	// The non-alpha is not read and not extracted
	// Result is stored in the symbol parameter
	// return value of FALSE means no symbol was found (end-of-file was found instead)
	static BOOL ReadSymbol(std::ifstream* stream, std::string* symbol)
	{
		int input_char = 0;		// buffer for one character
		symbol->clear();

		if (stream->peek() == -1)			// If the very next thing is the end
			return FALSE;					// then return false

		while (isalpha (stream->peek()) )			// if the next character is alphabetic
		{
			(*symbol) += (char)stream->get();		// then add that character to the string
			if (stream->peek() == -1)				// if the next thing is an end-of-file,
				return TRUE;						// then we're done
		}											// and repeat

		// reaching this point means we've encountered a non-alphabetic character
		return TRUE;
	}



	// Read a keyboard name, such as \\?\ACPI#PNP0303#4&231b8599&0#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}
	// and store it in kb's device_name field
	static BOOL ReadKeyboardName(std::ifstream* stream, KEYBOARD* kb)
	{
		// from <codecvt>, the object that will convert between narrow and wide strings (C++11 and newer)
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		// Because we read in UTF-8, but device names are stored internally as WCHARs.

		auto buffer = std::string();					// utf-8
		auto wideBuffer = std::wstring();				// utf-16

		(*stream) >> buffer;							// read name, stop at whitespace
		if (stream->eof()) return FALSE;	// kb name wan't there
		wideBuffer = converter.from_bytes(buffer);			// convert and then store it in the wide buffer
		swprintf_s(kb->device_name, kb->device_name_sizeof, L"%ls", wideBuffer.c_str());
		return TRUE;
	}


	static BOOL ReadModifierHex(std::ifstream* stream, BYTE* modifiers, UINT32* code)
	{
		int input_char = 0;		// buffer for one character

		int sideModifier = 0;	// 0: next modifier applies to left and right
								// 1: left modifier
								// 2: right modifier

		input_char = stream->peek();		// evaluate *next* character
		switch (input_char)
		{
		case '<':					// next modifier is left only
			sideModifier = 1;
			stream->get();
			break;

		case '>':					// next modifier is right only
			sideModifier = 2;
			stream->get();
			break;

		case '#':					// Winkey (unaffected by side modifiers)
			if (sideModifier == 1 || sideModifier == 0)
				*modifiers |= MODIFIER_LWIN;
			if (sideModifier == 2 || sideModifier == 0)
				*modifiers |= MODIFIER_RWIN;		// (I've never seen a right Winkey)
			sideModifier = 0;
			stream->get();
			break;

		case '^':					// Ctrl
			if (sideModifier == 1 || sideModifier == 0)
				*modifiers |= MODIFIER_LCTRL;
			if (sideModifier == 2 || sideModifier == 0)
				*modifiers |= MODIFIER_RCTRL;
			sideModifier = 0;
			stream->get();
			break;

		case '!':					// Alt
			if (sideModifier == 1 || sideModifier == 0)
				*modifiers |= MODIFIER_LALT;
			if (sideModifier == 2 || sideModifier == 0)
				*modifiers |= MODIFIER_RALT;
			sideModifier = 0;
			stream->get();
			break;

		case '+':					// Shift
			if (sideModifier == 1 || sideModifier == 0)
				*modifiers |= MODIFIER_LSHIFT;
			if (sideModifier == 2 || sideModifier == 0)
				*modifiers |= MODIFIER_RSHIFT;
			sideModifier = 0;
			stream->get();
			break;

		case '0':
		{	// brackets for locality
			// expect an 'x' or 'X'
			stream->get();
			input_char = stream->peek();
			if (input_char != 'x' && input_char != 'X')
				return FALSE;
			stream->get();
			// while the next character is a hexadecimal digit, read it
			std::string numberBuffer = std::string();
			while (isxdigit(stream->peek()))
			{
				numberBuffer += stream->get();
			}
			if (sscanf_s(numberBuffer.c_str(), "%x", code) != 1)
				return FALSE;		// that's one way to cast
									// we're not trying to be fast, exactly
									// scanf returns the amount of variables scanned
			else return TRUE;
		}

		default:			// Do not extract character
			return FALSE;
		}
	}


	// Read a trigger, such as <+<!0x47
	// (modifiers and a scancode)
	// and place it into a trigger structure
	// Returns FALSE upon encountering an unrecognized character
	static BOOL ReadTrigger(std::ifstream* stream, KEYSTROKE_INPUT* trigger)
	{
		UINT32 code;
		BOOL result = ReadModifierHex(stream, &(trigger->modifiers), &code);
		trigger->scancode = (USHORT)code;			// lossy cast, but shouldn't lose any data
		return result;
	}



	// Read a virtual key, together with modifiers, such as <^0x41
	// Maybe we'll support multiple keypresses in the future, but not now.
	static BOOL ReadVirtualkey(std::ifstream* stream, KEYSTROKE_OUTPUT* keystroke)
	{
		return ReadModifierHex(stream, &(keystroke->modifiers), &(keystroke->codepoint));
	}



	// Expects a hexadecimal value
	static BOOL ReadUnicode(std::ifstream* stream, KEYSTROKE_OUTPUT* keystroke)
	{
		// All modifiers will be read but ignored
		while (stream->peek() == '<'
			|| stream->peek() == '>'
			|| stream->peek() == '#'
			|| stream->peek() == '^'
			|| stream->peek() == '!'
			|| stream->peek() == '+')
			stream->get();

		if (stream->peek() != '0')			// expect a hexadecimal
			return FALSE;
		stream->get();
		if (stream->peek() != 'x' && stream->peek() != 'X')
			return FALSE;
		stream->get();

		std::string numberBuffer = std::string();
		while (isxdigit(stream->peek()))		// read all following hex digits
		{
			numberBuffer += stream->get();
		}
		if (sscanf_s(numberBuffer.c_str(), "%x", &(keystroke->codepoint)) != 1)
			return FALSE;

		return TRUE;

	}


public:

	// This isn't an interpreter.
	static BOOL ReadFile(std::string filename, std::unordered_map<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT> * map)
	{
		// having a function explicitly expect such a specific map is not very nice, but that's what I could do.

		// Also, in the future we should consider supporting UTF-16 filenames. I don't know how to do that.
		
		setlocale(LC_ALL, "");						// Set the locale, just in case
		
		std::ifstream file(filename.c_str());		// Input file stream because we only need to read

		if (!file.is_open())
			return FALSE;							// oh, no!
		
		map->clear();								// First of all, clear the map

		
		// hold one line
		std::string lineBuffer;

		// hold one symbol
		std::string symbol;

		// hold the current keyboard being written to
		KEYBOARD keyboard;


		file >> symbol;				// first line of file must open a keyboard
		if (symbol != "keyboard")
			return FALSE;

		// Now we can enter the loop.
		while (file >> symbol)				// doen't work for unicode(<+<!0x048...
		{
			getline(file, lineBuffer);
			if (file.eof()) return TRUE;	// expected end of file
			
		}


	}
};




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





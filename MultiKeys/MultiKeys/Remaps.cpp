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
	std::ifstream file(filename.c_str());		// Open file with ANSI filename

	if (!file.is_open())
		return FALSE;

	BOOL result = Parser::ReadFile(&file, &keyboards);
	file.close();
	return result;
}
BOOL Multikeys::Remapper::LoadSettings(std::wstring filename)
{
	std::ifstream file(filename.c_str());		// Open file with unicode (UTF-16) filename

	if (!file.is_open())
		return FALSE;

	BOOL result = Parser::ReadFile(&file, &keyboards);
	file.close();
	return result;
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








/*----Parser static class----*/

BOOL Multikeys::Parser::ReadSymbol(std::ifstream* stream, std::string* symbol)
{
	int input_char = 0;		// buffer for one character
	symbol->clear();

	if (stream->peek() == -1)			// If the very next thing is the end
		return FALSE;					// then return false

	while (isalpha(stream->peek()))			// if the next character is alphabetic
	{
		(*symbol) += (char)stream->get();		// then add that character to the string
		if (stream->peek() == -1)				// if the next thing is an end-of-file,
			return TRUE;						// then we're done
	}											// and repeat

												// reaching this point means we've encountered a non-alphabetic character
	return TRUE;
}


BOOL Multikeys::Parser::ReadKeyboardName(std::ifstream* stream, KEYBOARD* kb)
{
	// from <codecvt>, the object that will convert between narrow and wide strings (C++11 and newer)
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	// Because we read in UTF-8, but device names are stored internally as WCHARs.

	auto buffer = std::string();					// utf-8
	auto wideBuffer = std::wstring();				// utf-16

	while (!isspace(stream->peek()) && stream->peek() != ')')
	{
		buffer += stream->get();
	}
	if (buffer == "") return FALSE;		// kb name wasn't there
	wideBuffer = converter.from_bytes(buffer);			// convert and then store it in the wide buffer
	swprintf_s(kb->device_name, kb->device_name_sizeof, L"%ls", wideBuffer.c_str());
	return TRUE;
}


BOOL Multikeys::Parser::ReadModifierHex(std::ifstream* stream, BYTE* modifiers, UINT32* code)
{
	int input_char = 0;		// buffer for one character

	int sideModifier = 0;	// 0: next modifier applies to left and right
							// 1: left modifier
							// 2: right modifier

	(*modifiers) = 0;		// clearing...
	(*code) = 0;

	input_char = stream->peek();		// evaluate *next* character
	while (true)
	{
		switch (input_char)					// TODO: Put this switch into a loop
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
		}	// end of switch
	}	// loop
}


BOOL Multikeys::Parser::ReadInputKeystroke(std::ifstream* stream, KEYSTROKE_INPUT* trigger)
{
	UINT32 code = 0;
	BOOL result = ReadModifierHex(stream, &(trigger->modifiers), &code);
	trigger->scancode = (USHORT)code;			// lossy cast, but shouldn't lose any data
	return result;
}


BOOL Multikeys::Parser::ReadVirtualkey(std::ifstream* stream, KEYSTROKE_OUTPUT* keystroke)
{
	keystroke->flags = 0;
	return ReadModifierHex(stream, &(keystroke->modifiers), &(keystroke->codepoint));
}


BOOL Multikeys::Parser::ReadUnicode(std::ifstream* stream, KEYSTROKE_OUTPUT* keystroke)
{
	keystroke->flags = KEYEVENTF_UNICODE;

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


BOOL Multikeys::Parser::ReadFile(std::ifstream* stream, std::vector<KEYBOARD> * ptrVectorKeyboard)
{
	// having a function explicitly expect such a specific map is not very nice, but that's what I could do.

	// Also, in the future we should consider supporting UTF-16 filenames. I don't know how to do that.

	setlocale(LC_ALL, "");						// Set the locale, just in case


	ptrVectorKeyboard->clear();					// clear all keyboards with the maps in them


												// hold one line
	auto lineBuffer = std::string();

	// hold one symbol
	auto symbol = std::string();

	// hold the current keyboard being written to
	auto keyboard = KEYBOARD();

	// input and output
	auto input = KEYSTROKE_INPUT();
	auto output = KEYSTROKE_OUTPUT();

	// one character
	int read_char = 0;


	// First term should be a "keyboard"
	if (!ReadSymbol(stream, &symbol)) return FALSE;
	if (symbol != "keyboard") return FALSE;
	// expect left parenthesis
	read_char = stream->get();
	if (read_char != '(') return FALSE;
	// read keyboard name
	if (!ReadKeyboardName(stream, &keyboard)) return FALSE;
	// expect right parenthesis
	read_char = stream->get();
	if (read_char != ')') return FALSE;
	// jump to next line
	getline(*stream, lineBuffer);
	// begin loop
	while (true)
	{
		if (!ReadSymbol(stream, &symbol))
		{
			// save keyboard and close
			ptrVectorKeyboard->push_back(keyboard);
			return TRUE;
		}
		// evaluate symbol
		if (symbol == "unicode")
		{
			// case 1 - unicode
			// expect a left parenthesis
			read_char = stream->get();
			if (read_char != '(') return FALSE;
			// first parameter: an input keystroke
			if (!ReadInputKeystroke(stream, &input)) return FALSE;
			// expect a comma
			read_char = stream->get();
			if (read_char != ',') return FALSE;
			// second parameter: an output keystroke with a unicode codepoint
			if (!ReadUnicode(stream, &output)) return FALSE;
			// expect a right parenthesis
			read_char = stream->get();
			if (read_char != ')') return FALSE;
			// place input and output into current keyboard
			keyboard.remaps.insert((std::pair<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT>(input, output)));
		}
		else if (symbol == "virtual")
		{
			// case 2 - virtual key
			// expect a left parenthesis
			read_char = stream->get();
			if (read_char != '(') return FALSE;
			// first parameter: an input keystroke
			if (!ReadInputKeystroke(stream, &input)) return FALSE;
			// expect a comma
			read_char = stream->get();
			if (read_char != ')') return FALSE;
			// second parameter: an output keystroke with a virtual key
			if (!ReadVirtualkey(stream, &output)) return FALSE;
			// expect a right parenthesis
			read_char = stream->get();
			if (read_char != ')') return FALSE;
			// place input and output into current keyboard
			keyboard.remaps.insert((std::pair<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT>(input, output)));
		}
		else if (symbol == "keyboard")
		{
			// case 3 - new keyboard
			// save this keyboard
			ptrVectorKeyboard->push_back(keyboard);
			// read a new one=
			if (!ReadSymbol(stream, &symbol)) return FALSE;
			if (symbol != "keyboard") return FALSE;
			// expect left parenthesis
			read_char = stream->get();
			if (read_char != '(') return FALSE;
			// read keyboard name
			if (!ReadKeyboardName(stream, &keyboard)) return FALSE;
			// expect right parenthesis
			read_char = stream->get();
			if (read_char != ')') return FALSE;
			// jump to next line
			getline(*stream, lineBuffer);
		}
		// jump line
		getline(*stream, lineBuffer);

	}	// loop
}
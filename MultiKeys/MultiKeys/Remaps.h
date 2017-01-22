#pragma once

#include "stdafx.h"

#include "InputSimulator.h"


// Only the remapper and the parser need to know about this.
// Used to internally store a group of remaps associated to a specific keyboard
struct KEYBOARD
{
	// Name of this device
	WCHAR * device_name;

	// Size of buffer
	USHORT const device_name_sizeof = 128;

	// The simulator that sends keystrokes

	// Map between inputs and outputs
	std::unordered_map<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT> remaps;

	KEYBOARD()
	{
		device_name = new WCHAR[device_name_sizeof];
	}
};


namespace Multikeys
{
	
	class Remapper
	{
	private:
		// work variable
		WCHAR* wcharWork;

		// a vector of keyboards
		std::vector<KEYBOARD> keyboards;
		// to hold each keyboard
		// It would be possible to make a map between keyboard name and keyboard
		// but we won't have that many at once to require such a thing

		// Loads the settings in file at filename into memory
		BOOL LoadSettings(std::string filename);

		// Class that will simulate the output for this remapper
		InputSimulator inputSimulator;

	public:
		
		// Constructor
		// Parameters:
		//		std::string - full path to the configuration file to be read
		Remapper(std::string filename);

		// EvaluateKey
		//
		// Receives information about a keypress, analyses it, then if there is a remap,
		//		returns information about the remapped input.
		//
		// Parameters:
		//		RAWKEYBOARD* keypressed - pointer to the structure representing the keyboard input,
		//				from the Raw Input API.
		//		WCHAR* deviceName - wide string containing the name of the device that generated the keystroke
		//		KEYSTROKE_OUTPUT* out_action - if there is a remap for the keystroke in RAWKEYBOARD* keypressed
		//				for the keyboard of name WCHAR* deviceName, then this will point to a
		//				KEYSTROKE_OUTPUT containing the keystroke or unicode code point to be simulated.
		// Return value:
		//		TRUE - There is a remap, and it's been placed in *out_action
		//		FALSE - There is no remap for this key.
		BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, KEYSTROKE_OUTPUT* out_action);


		BOOL SimulateKeystroke(KEYSTROKE_OUTPUT key);

		// ReloadSettings
		//
		// Replace all remaps in memory with the settings in file at filename.
		//
		// Parameters:
		//		std::string - full path to the configuration file to be read
		// Return value:
		//		TRUE - Reload succeeded
		//		FALSE - Reload was unsuccessful
		BOOL ReloadSettings(std::string filename);


	};

	
}



void ParserTest();
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

		while (isalpha(stream->peek()))			// if the next character is alphabetic
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
	// Will only stop at whitespace (including newline) or a right parenthesis
	static BOOL ReadKeyboardName(std::ifstream* stream, KEYBOARD* kb)
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


	static BOOL ReadModifierHex(std::ifstream* stream, BYTE* modifiers, UINT32* code)
	{
		int input_char = 0;		// buffer for one character

		int sideModifier = 0;	// 0: next modifier applies to left and right
								// 1: left modifier
								// 2: right modifier

		(*modifiers) = 0;		// clearing...
		(*code) = 0;

		input_char = stream->peek();		// evaluate *next* character
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
		}

		// switch ran out?
		return FALSE;
	}


	// Read an input keystroke, such as <+<!0x47
	// (modifiers and a scancode)
	// and place it into a trigger structure
	// Returns FALSE upon encountering an unrecognized character
	static BOOL ReadInputKeystroke(std::ifstream* stream, KEYSTROKE_INPUT* trigger)
	{
		UINT32 code = 0;
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
	static BOOL ReadFile(std::string filename, std::vector<KEYBOARD> * ptrVectorKeyboard)
	{
		// having a function explicitly expect such a specific map is not very nice, but that's what I could do.

		// Also, in the future we should consider supporting UTF-16 filenames. I don't know how to do that.

		setlocale(LC_ALL, "");						// Set the locale, just in case

		std::ifstream file(filename.c_str());		// Input file stream because we only need to read

		if (!file.is_open())
			return FALSE;							// oh, no!

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
		if (!ReadSymbol(&file, &symbol)) return FALSE;
		if (symbol != "keyboard") return FALSE;
		// expect left parenthesis
		read_char = file.get();
		if (read_char != '(') return FALSE;
		// read keyboard name
		if (!ReadKeyboardName(&file, &keyboard)) return FALSE;
		// expect right parenthesis
		read_char = file.get();
		if (read_char != ')') return FALSE;
		// jump to next line
		getline(file, lineBuffer);
		// begin loop
		while (true)
		{
			if (!ReadSymbol(&file, &symbol))
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
				read_char = file.get();
				if (read_char != '(') return FALSE;
				// first parameter: an input keystroke
				if (!ReadInputKeystroke(&file, &input)) return FALSE;
				// expect a comma
				read_char = file.get();
				if (read_char != ',') return FALSE;
				// second parameter: an output keystroke with a unicode codepoint
				if (!ReadUnicode(&file, &output)) return FALSE;
				// expect a right parenthesis
				read_char = file.get();
				if (read_char != ')') return FALSE;
				// place input and output into current keyboard
				keyboard.remaps.insert((std::pair<KEYSTROKE_INPUT, KEYSTROKE_OUTPUT>(input, output)));
			}
			else if (symbol == "virtual")
			{
				// case 2 - virtual key
				// expect a left parenthesis
				read_char = file.get();
				if (read_char != '(') return FALSE;
				// first parameter: an input keystroke
				if (!ReadInputKeystroke(&file, &input)) return FALSE;
				// expect a comma
				read_char = file.get();
				if (read_char != ')') return FALSE;
				// second parameter: an output keystroke with a virtual key
				if (!ReadVirtualkey(&file, &output)) return FALSE;
				// expect a right parenthesis
				read_char = file.get();
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
				if (!ReadSymbol(&file, &symbol)) return FALSE;
				if (symbol != "keyboard") return FALSE;
				// expect left parenthesis
				read_char = file.get();
				if (read_char != '(') return FALSE;
				// read keyboard name
				if (!ReadKeyboardName(&file, &keyboard)) return FALSE;
				// expect right parenthesis
				read_char = file.get();
				if (read_char != ')') return FALSE;
				// jump to next line
				getline(file, lineBuffer);
			}
			// jump line
			getline(file, lineBuffer);

		}	// loop
	}
};
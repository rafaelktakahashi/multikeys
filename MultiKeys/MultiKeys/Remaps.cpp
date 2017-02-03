#include "stdafx.h"

#include "Remaps.h"












class Parser
{
public:


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
		return FALSE;
		/*
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
		*/
	}













	static BOOL ReadInputKeystroke(std::ifstream* stream, KEYSTROKE_INPUT* trigger)
	{
		UINT32 code = 0;
		BOOL result = ReadModifierHex(stream, &(trigger->modifiers), &code);
		trigger->scancode = (USHORT)code;			// lossy cast, but shouldn't lose any data
		return result;
	}


	static BOOL ReadVirtualkey(std::ifstream* stream, IKeystrokeOutput* keystroke)
	{
		//keystroke->flags = 0;
		//return ReadModifierHex(stream, &(keystroke->modifiers), &(keystroke->codepoint));
	}


	static BOOL ReadUnicode(std::ifstream* stream, IKeystrokeOutput* keystroke)
	{
		/*
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
		*/
	}




	static BOOL ReadFile(std::ifstream* stream, std::vector<KEYBOARD> * ptrVectorKeyboard)
	{
		// Make a dummy object for now to test other things

		// Have this file generate the data structure, and build the parser later.
		ptrVectorKeyboard->clear();

		auto keyboard = KEYBOARD();

		keyboard.device_name = L"\\\\?\\HID#VID_0510&PID_0002#7&141e5925&0&0000#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}";
		keyboard.remaps.clear();


		auto input = KEYSTROKE_INPUT();

		input.scancode = 0x02;
		input.flags = 0;
		input.modifiers = 0;

		auto output = new UnicodeOutput();
		output->codepoint = 0x1f604;

		INPUT * oneSurrogate = new INPUT[2]();
		INPUT * oneSurrogateUp = new INPUT[2]();

		
		oneSurrogate[0].type = INPUT_KEYBOARD;
		oneSurrogate[0].ki.dwExtraInfo = 0;
		oneSurrogate[0].ki.dwFlags = KEYEVENTF_UNICODE;
		oneSurrogate[0].ki.wScan = 0xd83d;
		oneSurrogate[0].ki.wVk = 0;
		oneSurrogate[0].ki.time = 0;

		oneSurrogate[1].type = INPUT_KEYBOARD;
		oneSurrogate[1].ki.dwExtraInfo = 0;
		oneSurrogate[1].ki.dwFlags = KEYEVENTF_UNICODE;
		oneSurrogate[1].ki.wScan = 0xde04;
		oneSurrogate[1].ki.wVk = 0;
		oneSurrogate[1].ki.time = 0;

		oneSurrogateUp[0].type = INPUT_KEYBOARD;
		oneSurrogateUp[0].ki.dwExtraInfo = 0;
		oneSurrogateUp[0].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
		oneSurrogateUp[0].ki.wScan = 0xd83d;
		oneSurrogateUp[0].ki.wVk = 0;
		oneSurrogateUp[0].ki.time = 0;

		oneSurrogateUp[1].type = INPUT_KEYBOARD;
		oneSurrogateUp[1].ki.dwExtraInfo = 0;
		oneSurrogateUp[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
		oneSurrogateUp[1].ki.wScan = 0xde04;
		oneSurrogateUp[1].ki.wVk = 0;
		oneSurrogateUp[1].ki.time = 0;

		output->keystrokesDown = oneSurrogate;
		output->keystrokesUp = oneSurrogateUp;
		output->inputCount = 2;

		keyboard.remaps.insert(std::pair<KEYSTROKE_INPUT, IKeystrokeOutput*>(input, output));

		ptrVectorKeyboard->push_back(keyboard);

		return TRUE;

		/*
		// having a function explicitly expect such a specific map is not very nice, but that's what I could do.

		// Also, in the future we should consider supporting UTF-16 filenames. I don't know how to do that. // <- that's done

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
		auto output = IKeystrokeOutput();

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
		keyboard.remaps.insert((std::pair<KEYSTROKE_INPUT, IKeystrokeOutput>(input, output)));
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
		keyboard.remaps.insert((std::pair<KEYSTROKE_INPUT, IKeystrokeOutput>(input, output)));
		}
		else if (symbol == "keyboard")
		{
		// case 3 - new keyboard
		// save this keyboard
		ptrVectorKeyboard->push_back(keyboard);		// store a copy
		// expect left parenthesis
		read_char = stream->get();
		if (read_char != '(') return FALSE;
		// clean keyboard
		keyboard.Clear();
		// read keyboard name
		if (!ReadKeyboardName(stream, &keyboard)) return FALSE;
		// expect right parenthesis
		read_char = stream->get();
		if (read_char != ')') return FALSE;
		}
		// jump line
		getline(*stream, lineBuffer);

		}	// loop
		*/


	}



};






















/*
Multikeys::Remapper::Remapper(std::string filename)					// constructor
{
	// Setup a keyboard input simulator
	// inputSimulator = InputSimulator();

	if (!LoadSettings(filename))
	{
		
	}
	OutputDebugString(L"Initialized.");
}
Multikeys::Remapper::Remapper(std::wstring filename)
{
	// Setup a keyboard input simulator
	// inputSimulator = InputSimulator();

	if (!LoadSettings(filename))
	{ }
	OutputDebugString(L"Initialized.");
} */
Multikeys::Remapper::Remapper()
{
}




BOOL Multikeys::Remapper::LoadSettings(std::string filename)		// parser
{
	std::ifstream file(filename);		// Open file with ANSI filename

	if (!file.is_open())
		return FALSE;

	BOOL result = Parser::ReadFile(&file, &keyboards);
	file.close();
	return result;
}
BOOL Multikeys::Remapper::LoadSettings(std::wstring filename)
{
	std::ifstream file(filename);		// Open file with unicode (UTF-16) filename

	if (!file.is_open())
		return FALSE;

	BOOL result = Parser::ReadFile(&file, &keyboards);
	file.close();
	return result;
}







BOOL Multikeys::Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, IKeystrokeOutput ** out_action)
{
	// Make an input

	// Need to deal with modifiers. Not here, the levels thing.
	KEYSTROKE_INPUT input = KEYSTROKE_INPUT(0, keypressed->MakeCode, keypressed->Flags);
	// We'll look for a similar one in our list
	
	// Look for correct device; return FALSE (= do not block) otherwise
	for (auto iterator = keyboards.begin(); iterator != keyboards.end(); iterator++)
	{
		if (wcscmp(iterator->device_name, deviceName) == 0)
		{
			// found it!
			// check if the remaps map for this device contains our scancode
			auto pairIterator = iterator->remaps.find(input);
			if (pairIterator != iterator->remaps.end())
			{
				*out_action = pairIterator->second;
				return TRUE;
			}
			else
				return FALSE;
			
		}
	}

	return FALSE;
}







// Experimental: factory class for outputs
class OutputFactory
{
private:
	INPUT UnicodeTemplateDown;
	INPUT UnicodeTemplateUp;

	INPUT VirtualKeyTemplateDown;
	INPUT VirtualKeyTemplateUp;

public:

	OutputFactory()
	{
		UnicodeTemplateDown.type = INPUT_KEYBOARD;
		UnicodeTemplateDown.ki.dwExtraInfo = 0;
		UnicodeTemplateDown.ki.dwFlags = KEYEVENTF_UNICODE;
		UnicodeTemplateDown.ki.time = 0;
		UnicodeTemplateDown.ki.wVk = 0;

		UnicodeTemplateUp = UnicodeTemplateDown;
		UnicodeTemplateUp.ki.dwFlags |= KEYEVENTF_KEYUP;

		VirtualKeyTemplateDown.type = INPUT_KEYBOARD;
		VirtualKeyTemplateDown.ki.dwExtraInfo = 0;
		VirtualKeyTemplateDown.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
		VirtualKeyTemplateDown.ki.time = 0;
		VirtualKeyTemplateDown.ki.wScan = 0;

		VirtualKeyTemplateUp = VirtualKeyTemplateDown;
		VirtualKeyTemplateUp.ki.dwFlags |= KEYEVENTF_KEYUP;
	}

	// Parameter UINT_PTR scales to the size of a pointer (32-bit or 64-bit)
	// If casting to an integer type is necessary, use ptrtolong
	IKeystrokeOutput * getInstance(KeystrokeOutputType _type, UINT _nParam, ULONG_PTR _lParam)
	{
		switch (_type)
		{
		case KeystrokeOutputType::UnicodeOutput:
		{
			// integer parameter contains the Unicode codepoint
			// second parameter contains nothing
			auto unicodeOutput = new UnicodeOutput();
			UINT codepoint = _nParam;

			if (codepoint <= 0xffff)
			{
				// One UTF-16 code unit

				unicodeOutput->inputCount = 1;

				unicodeOutput->keystrokesDown = new INPUT(UnicodeTemplateDown);
				unicodeOutput->keystrokesDown->ki.wScan = codepoint;

				unicodeOutput->keystrokesUp = new INPUT(UnicodeTemplateUp);
				unicodeOutput->keystrokesUp->ki.wScan = codepoint;

				return (IKeystrokeOutput*)&unicodeOutput;
			}
			else
			{
				// UTF-16 surrogate pair

				unicodeOutput->inputCount = 2;

				unicodeOutput->keystrokesDown = new INPUT[2];
				unicodeOutput->keystrokesUp = new INPUT[2];
				for (int i = 0; i < 2; i++) {
					unicodeOutput->keystrokesDown[i] = INPUT(UnicodeTemplateDown);
					unicodeOutput->keystrokesUp[i] = INPUT(UnicodeTemplateDown);
				}

				codepoint -= 0x10000;
				unicodeOutput->keystrokesDown[0].ki.wScan
					= unicodeOutput->keystrokesUp[0].ki.wScan
					= 0xd800 + (codepoint >> 10);			// High surrogate
				unicodeOutput->keystrokesDown[1].ki.wScan
					= unicodeOutput->keystrokesUp[1].ki.wScan
					= 0xdc00 + (codepoint & 0x3ff);			// Low surrogate

				return (IKeystrokeOutput*)&unicodeOutput;

			}
			
		}	// end case
		case KeystrokeOutputType::VirtualOutput:
		{
			// Integer parameter contains the virtual-key code
			// Second parameter contains flags for modifiers (only the last byte)
			// (These are not the modifiers that trigger a remap, but the ones used
			// for simulating shortcuts

			auto virtualKeyOutput = VirtualKeyOutput();
			
			// There are better ways to count the number of set bits
			// but come on
			USHORT modifierCount = 0;
			virtualKeyOutput.inputCount = modifierCount + 1;
			for (int i = 0; i < 8; i++)
				if ((_lParam >> i) & 1)
					modifierCount++;
			// Above code doesn't modify value of _lParam

			// We need one INPUT for each modifier, plus one for the key itself
			USHORT currentIndex = 0;
			virtualKeyOutput.keystrokesDown = new INPUT[modifierCount + 1];
			virtualKeyOutput.keystrokesUp = new INPUT[modifierCount + 1];
			
			// Careful - keystrokes up are in inverse order, with index 0 corresponding to the key itself
			if ((_lParam & VIRTUAL_MODIFIER_LCTRL) == VIRTUAL_MODIFIER_LCTRL)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_LCONTROL;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LCONTROL;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_RCTRL) == VIRTUAL_MODIFIER_RCTRL)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_RCONTROL;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RCONTROL;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_LALT) == VIRTUAL_MODIFIER_LALT)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_LMENU;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LMENU;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_RALT) == VIRTUAL_MODIFIER_RALT)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_RMENU;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RMENU;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_LWIN) == VIRTUAL_MODIFIER_LWIN)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_LWIN;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LWIN;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_RWIN) == VIRTUAL_MODIFIER_RWIN)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_RWIN;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RWIN;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_LSHIFT) == VIRTUAL_MODIFIER_RSHIFT)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_LSHIFT;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LSHIFT;
				currentIndex++;
			}
			if ((_lParam & VIRTUAL_MODIFIER_RSHIFT) == VIRTUAL_MODIFIER_RSHIFT)
			{
				virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = VK_RSHIFT;
				virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RSHIFT;
				currentIndex++;
			}
			virtualKeyOutput.keystrokesDown[currentIndex].ki.wVk = (WORD)_nParam;
			virtualKeyOutput.keystrokesUp[modifierCount - currentIndex].ki.wVk = (WORD)_nParam;
			
			return (IKeystrokeOutput*)&virtualKeyOutput;

		}	// end case

		case KeystrokeOutputType::MacroOutput:
		{
			// second parameter casts to a pointer to an array of {WORD, BOOL} containing the sequence
			// of keypresses
			// first parameter contains the lenght of array pointed to by second parameter.
			if (_lParam == 0) return nullptr;

			// Need to figure out a data structure for {WORD, BOOL} - maybe an std::pair?
			auto sequence = (std::pair<WORD, BOOL>*)_lParam;		// <- would this work fine?
			UINT seqCount = _nParam;

			auto macroOutput = new MacroOutput();

			




			return nullptr;
		}	// end case

		case KeystrokeOutputType::StringOutput:
		{
			// second parameter casts to a pointer to array of UINT containing all
			// the Unicode code points in string
			// first parameter contains the length of array pointed to by second parameter
			if (_lParam == 0) return nullptr;

			auto codepoints = (UINT*)_lParam;
			UINT seqCount = _nParam;

			auto stringOutput = new StringOutput();

			// number of sent INPUTs is the length of array, plus the amount of code points
			// that are larger than 0xffff
			// Then multiplied by 2 because both keydowns and keyups are sent
			stringOutput->inputCount = seqCount * 2;
			for (int i = 0; i < seqCount; i++)
			{
				if (codepoints[i] > 0xffff)
					stringOutput->inputCount += 2;
			}

			stringOutput->keystrokesDown = new INPUT[stringOutput->inputCount];

			// Put each character and surrogate pair into string
			// Here's a thought: when it's a surrogate pair, do both keyups have to happen
			// after both keydowns?
			// Do we even need keyups for unicode characters?

		}	// end case

		}


		return nullptr;
	}
};
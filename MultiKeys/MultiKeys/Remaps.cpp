#include "stdafx.h"

#include "Remaps.h"


// Factory class for outputs
class OutputFactory
{
private:

	// Prototypes, so that INPUTs don't need to be manually set everywhere

	INPUT UnicodePrototypeDown;
	INPUT UnicodePrototypeUp;

	INPUT VirtualKeyPrototypeDown;
	INPUT VirtualKeyPrototypeUp;

public:

	OutputFactory()
	{
		// Initialize prototypes

		// When keyeventf_unicode is set, virtual key must be 0,
		// and the UTF-16 code value is put into wScan
		// Surrogate pairs require two consecutive inputs
		UnicodePrototypeDown.type = INPUT_KEYBOARD;
		UnicodePrototypeDown.ki.dwExtraInfo = 0;
		UnicodePrototypeDown.ki.dwFlags = KEYEVENTF_UNICODE;
		UnicodePrototypeDown.ki.time = 0;
		UnicodePrototypeDown.ki.wVk = 0;

		UnicodePrototypeUp = UnicodePrototypeDown;
		UnicodePrototypeUp.ki.dwFlags |= KEYEVENTF_KEYUP;

		// Virtual keys are sent with the scancode e0 00 because the hook
		// will filter these out (to avoid responding to injected keys)
		VirtualKeyPrototypeDown.type = INPUT_KEYBOARD;
		VirtualKeyPrototypeDown.ki.dwExtraInfo = 0;
		VirtualKeyPrototypeDown.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
		VirtualKeyPrototypeDown.ki.time = 0;
		VirtualKeyPrototypeDown.ki.wScan = 0;

		VirtualKeyPrototypeUp = VirtualKeyPrototypeDown;
		VirtualKeyPrototypeUp.ki.dwFlags |= KEYEVENTF_KEYUP;
	}


	// ----USAGE----
	// The meaning of _nParam and _lParam change depending on _type:
	// type UnicodeOutput:
	//		_nParam contains the Unicode code point to be sent,
	//		_lParam is not used
	// type VirtualOutput:
	//		_nParam contains the virtual-key code to be simulated,
	//		_lParam contains modifier flags (up to eight) to be simulated as well
	// type MacroOutput:
	//		_lParam is a pointer to an array of DWORDs, each containing the virtual-key code
	//				to be sent, with the high bit (leftmost) on for keyups
	//		_nParam is the length of the array
	// type StringOutput:
	//		_lParam is a pointer to an array of UINTs, each containing the Unicode codepoint
	//				of the character to be sent
	//		_nParam is the length of the array
	// type ScriptOutput:
	//		_lParam is a pointer to a null-terminated string of WCHAR (WCHAR*) containing
	//				the full path to the file to execute
	//		_nParam is unused
	// type NoOutput uses no input, and generates a structure that performs no action when executed.
	IKeystrokeOutput * getInstance(KeystrokeOutputType _type, UINT _nParam, ULONG_PTR _lParam)
	{
		switch (_type)
		{
		case KeystrokeOutputType::UnicodeOutput:
		{
			// integer parameter contains the Unicode codepoint
			// second parameter contains nothing
			UnicodeOutput * unicodeOutput = new UnicodeOutput();
			UINT codepoint = _nParam;
			unicodeOutput->codepoint = codepoint;

			if (codepoint <= 0xffff)
			{
				// One UTF-16 code unit

				unicodeOutput->inputCount = 1;

				unicodeOutput->keystrokesDown = new INPUT(UnicodePrototypeDown);
				unicodeOutput->keystrokesDown->ki.wScan = codepoint;

				unicodeOutput->keystrokesUp = new INPUT(UnicodePrototypeUp);
				unicodeOutput->keystrokesUp->ki.wScan = codepoint;

				return (IKeystrokeOutput*)unicodeOutput;
			}
			else
			{
				// UTF-16 surrogate pair

				unicodeOutput->inputCount = 2;

				unicodeOutput->keystrokesDown = new INPUT[2];
				unicodeOutput->keystrokesUp = new INPUT[2];
				for (int i = 0; i < 2; i++) {
					unicodeOutput->keystrokesDown[i] = INPUT(UnicodePrototypeDown);
					unicodeOutput->keystrokesUp[i] = INPUT(UnicodePrototypeUp);
				}

				codepoint -= 0x10000;
				unicodeOutput->keystrokesDown[0].ki.wScan
					= unicodeOutput->keystrokesUp[0].ki.wScan
					= 0xd800 + (codepoint >> 10);			// High surrogate
				unicodeOutput->keystrokesDown[1].ki.wScan
					= unicodeOutput->keystrokesUp[1].ki.wScan
					= 0xdc00 + (codepoint & 0x3ff);			// Low surrogate

				return (IKeystrokeOutput*)unicodeOutput;

			}

		}	// end case
		case KeystrokeOutputType::VirtualOutput:
		{
			// Integer parameter contains the virtual-key code
			// Second parameter contains flags for modifiers (only the last byte)
			// (These are not the modifiers that trigger a remap, but the ones used
			// for simulating shortcuts)
			

			VirtualKeyOutput * virtualKeyOutput = new VirtualKeyOutput();

			// There are better ways to count the number of set bits
			// but come on
			USHORT modifierCount = 0;
			for (unsigned int i = 0; i < 8; i++)
				if ((_lParam >> i) & 1)
					modifierCount++;
			// Above code doesn't modify value of _lParam
			virtualKeyOutput->inputCount = modifierCount + 1;

			// We need one INPUT for each modifier, plus one for the key itself
			USHORT currentIndex = 0;
			virtualKeyOutput->keystrokesDown = new INPUT[modifierCount + 1];
			virtualKeyOutput->keystrokesUp = new INPUT[modifierCount + 1];
			for (int i = 0; i < virtualKeyOutput->inputCount; i++) {
				virtualKeyOutput->keystrokesDown[i] = INPUT(VirtualKeyPrototypeDown);
				virtualKeyOutput->keystrokesUp[i] = INPUT(VirtualKeyPrototypeUp);
			}

			// Careful - keystrokes up are in inverse order, with index 0 corresponding to the key itself
			{
				if ((_lParam & VIRTUAL_MODIFIER_LCTRL) == VIRTUAL_MODIFIER_LCTRL)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_LCONTROL;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LCONTROL;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_RCTRL) == VIRTUAL_MODIFIER_RCTRL)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_RCONTROL;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RCONTROL;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_LALT) == VIRTUAL_MODIFIER_LALT)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_LMENU;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LMENU;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_RALT) == VIRTUAL_MODIFIER_RALT)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_RMENU;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RMENU;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_LWIN) == VIRTUAL_MODIFIER_LWIN)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_LWIN;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LWIN;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_RWIN) == VIRTUAL_MODIFIER_RWIN)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_RWIN;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RWIN;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_LSHIFT) == VIRTUAL_MODIFIER_RSHIFT)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_LSHIFT;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_LSHIFT;
					currentIndex++;
				}
				if ((_lParam & VIRTUAL_MODIFIER_RSHIFT) == VIRTUAL_MODIFIER_RSHIFT)
				{
					virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = VK_RSHIFT;
					virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = VK_RSHIFT;
					currentIndex++;
				}
			}
			virtualKeyOutput->keystrokesDown[currentIndex].ki.wVk = (WORD)_nParam;
			virtualKeyOutput->keystrokesUp[modifierCount - currentIndex].ki.wVk = (WORD)_nParam;

			return (IKeystrokeOutput*)virtualKeyOutput;

		}	// end case

		case KeystrokeOutputType::MacroOutput:
		{
			// second parameter casts to a pointer to an array of DWORD containing the sequence
			//		of keypresses (each DWORD contains the virtual key code in the last byte, and
			//		the high bit set for keyups)
			// first parameter contains the lenght of array pointed to by second parameter.
			if (_lParam == 0) return nullptr;

			// Planned feature: Have this function accept a delay in ms for each keystroke

			// WORD: 16-bit
			// DWORD: 32-bit
			// I think the size of Microsoft words will stay that way until the nth end of the world.
			DWORD * sequence = (DWORD*)_lParam;
			UINT seqCount = _nParam;

			auto macroOutput = new MacroOutput();
			macroOutput->inputCount = seqCount;
			macroOutput->keystrokes = new INPUT[seqCount];

			BOOL keyup = 0;
			USHORT virtualKeyCode = 0;
			for (unsigned int i = 0; i < seqCount; i++)
			{
				keyup = (sequence[i] >> 31) & 1;
				virtualKeyCode = sequence[i] & 0xff;
				macroOutput->keystrokes[i] = INPUT(VirtualKeyPrototypeDown);
				macroOutput->keystrokes[i].ki.wVk = virtualKeyCode;
				if (keyup) macroOutput->keystrokes[i].ki.dwFlags |= KEYEVENTF_KEYUP;
			}
			return (IKeystrokeOutput*)macroOutput;

		}	// end case

		case KeystrokeOutputType::StringOutput:
		{
			// second parameter casts to a pointer to array of UINT containing all
			// the Unicode code points in string
			// first parameter contains the length of array pointed to by second parameter
			if (_lParam == 0) return nullptr;

			UINT * codepoints = (UINT*)_lParam;
			UINT seqCount = _nParam;

			auto stringOutput = new StringOutput();

			// number of sent INPUTs is the length of array, plus the amount of code points
			//		that are larger than 0xffff
			// Then multiplied by 2 because both keydowns and keyups are sent
			stringOutput->inputCount = seqCount * 2;
			for (unsigned int i = 0; i < seqCount; i++)
			{
				if (codepoints[i] > 0xffff)
					stringOutput->inputCount += 2;
			}

			stringOutput->keystrokes = new INPUT[stringOutput->inputCount];



			// Put each character and surrogate pair into array of inputs
			UINT currentIndex = 0;

			stringOutput->keystrokes = new INPUT[stringOutput->inputCount];

			for (unsigned int i = 0; i < seqCount; i++)
			{
				// codepoints = one codepoint per character
				// seqCount = number of characters
				// stringOutput->inputCount = number of UTF-16 code values
				// currentIndex must be incremented manually

				if (codepoints[i] <= 0xffff)
				{
					// one UTF-16 code value, one input down, one input up
					stringOutput->keystrokes[currentIndex] = INPUT(UnicodePrototypeDown);
					stringOutput->keystrokes[currentIndex].ki.wScan = codepoints[i];
					currentIndex++;
					stringOutput->keystrokes[currentIndex] = INPUT(UnicodePrototypeUp);
					stringOutput->keystrokes[currentIndex].ki.wScan = codepoints[i];
					currentIndex++;
					continue;		// next for
				}
				else
				{
					// UTF-16 surrogate pair, two inputs down, two inputs up
					USHORT highSurrogate = 0xd800 + ((codepoints[i] - 0x10000) >> 10);
					USHORT lowSurrogate = 0xdc00 + (codepoints[i] & 0x3ff);
					stringOutput->keystrokes[currentIndex] = INPUT(UnicodePrototypeDown);
					stringOutput->keystrokes[currentIndex + 1] = INPUT(UnicodePrototypeDown);
					stringOutput->keystrokes[currentIndex].ki.wScan = highSurrogate;
					stringOutput->keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
					currentIndex += 2;
					stringOutput->keystrokes[currentIndex] = INPUT(UnicodePrototypeUp);
					stringOutput->keystrokes[currentIndex + 1] = INPUT(UnicodePrototypeUp);
					stringOutput->keystrokes[currentIndex].ki.wScan = highSurrogate;
					stringOutput->keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
					currentIndex += 2;
					continue;
				}
			}	// end for
			return (IKeystrokeOutput*)stringOutput;

		}	// end case


		case KeystrokeOutputType::ScriptOutput:
		{
			// second parameter contains a pointer to null-terminated WCHAR string containing filename to executable
			// first parameter contains nothing

			ScriptOutput * scriptOutput = new ScriptOutput((WCHAR*)_lParam);
			return (IKeystrokeOutput*)scriptOutput;
		}	// end case

		
		case KeystrokeOutputType::NoOutput:
		{
			// No inputs
			NoOutput * noOutput = new NoOutput();
			return (IKeystrokeOutput*)noOutput;
		}	// end case


		default: return nullptr;
		}	// end switch


	}
};












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
		// swprintf_s(kb->device_name, kb->device_name_sizeof, L"%ls", wideBuffer.c_str());
		return TRUE;
	}







	static BOOL ReadFile(std::ifstream* stream, std::vector<KEYBOARD> * ptrVectorKeyboard)
	{
		// Make a dummy object for now to test other things

		// Have this file generate the data structure, and build the parser later.

		// Yeah, this is pretty much working as a test class
		// because we don't have a proper test setup
		// sorry
		ptrVectorKeyboard->clear();

		KEYBOARD keyboard = KEYBOARD();

		keyboard.deviceName = L"\\\\?\\HID#VID_0510&PID_0002#7&141e5925&0&0000#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}";
		keyboard.addModifier(VK_RSHIFT, FALSE);
		keyboard.addModifier(VK_LSHIFT, FALSE);
		


		Level level;
		level.setModifiers2();

		OutputFactory factory = OutputFactory();


		auto pointerToAnotherOutput = factory.getInstance(KeystrokeOutputType::UnicodeOutput, 0x1f3b5, NULL);

		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x02, pointerToAnotherOutput));

		auto pointerToYetAnotherOutputButThisTimeItsAVirtualOutput =
			factory.getInstance(KeystrokeOutputType::VirtualOutput, 0x020, 0);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(0x03, pointerToYetAnotherOutputButThisTimeItsAVirtualOutput));

		DWORD macro[4];
		macro[0] = VK_LCONTROL;
		macro[1] = 0x46;
		macro[2] = 0x46 | 0x80000000;
		macro[3] = VK_LCONTROL | 0x80000000;

		auto pointerThisOneIsAMacro = factory.getInstance(KeystrokeOutputType::MacroOutput, 4, (ULONG_PTR)&macro);

		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x04, pointerThisOneIsAMacro));



		UINT unicodeString[4];
		unicodeString[0] = 0x48;
		unicodeString[1] = 0x69;
		unicodeString[2] = 0x2e;
		unicodeString[3] = 0x1f642;		// prints "Hi." and a smiling emoji.
		auto pointerStringOfUnicode = factory.getInstance(KeystrokeOutputType::StringOutput, 4, (ULONG_PTR)&unicodeString);

		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x05, pointerStringOfUnicode));


		auto wideStringFilename = L"C:\\MultiKeys\\openAppTest.exe";
		auto pointerThisOneWillOpenAnExecutableWhichOpensChromeIDontCareIfEdgeIsFasterChromeIsStillBetter
			= factory.getInstance(KeystrokeOutputType::ScriptOutput, 0, (ULONG_PTR)wideStringFilename);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(0x06, pointerThisOneWillOpenAnExecutableWhichOpensChromeIDontCareIfEdgeIsFasterChromeIsStillBetter));


		DWORD macro2[4];
		macro2[0] = VK_LWIN;
		macro2[1] = VK_SPACE;
		macro2[2] = VK_SPACE | 0x80000000;
		macro2[3] = VK_LWIN | 0x80000000;
		auto pointerToThingThatWillChangeLanguageWithLeftControlAndSpaceBar =
			factory.getInstance(KeystrokeOutputType::MacroOutput, 4, (ULONG_PTR)&macro2);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(0x07, pointerToThingThatWillChangeLanguageWithLeftControlAndSpaceBar));

		

		auto pointerToOneUnicodePerson = factory.getInstance(KeystrokeOutputType::UnicodeOutput, 0x1f468, NULL);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x08, pointerToOneUnicodePerson));

		

		keyboard.levels.push_back(level);

		// Add another level:
		
		
		level.setModifiers2(2, 2);		// LShift, RShift, either
		level.layout.clear();

		auto pointerToShiftedUnicode = factory.getInstance(KeystrokeOutputType::UnicodeOutput, 0x1f3b6, NULL);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x02, pointerToShiftedUnicode));

		/*U+1F468 U+200D U+1F469 U+200D U+1F467 U+200D U+1F467 is family emoji*/
		UINT unicodePeople[7];
		unicodePeople[0] = 0x1f468;
		unicodePeople[1] = 0x200d;
		unicodePeople[2] = 0x1f469;
		unicodePeople[3] = 0x200d;
		unicodePeople[4] = 0x1f467;
		unicodePeople[5] = 0x200d;
		unicodePeople[6] = 0x1f467;
		auto pointerToManyUnicodePeople = factory.getInstance(KeystrokeOutputType::StringOutput, 7, (ULONG_PTR)&unicodePeople);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x08, pointerToManyUnicodePeople));

		keyboard.levels.push_back(level);


		

		keyboard.resetModifierState();

		ptrVectorKeyboard->push_back(keyboard);

		return TRUE;



	}



};





















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
	keyboards[0].resetModifierState();		// I do not fully comprehend why this line is necessary
										// it fixes the pointer to the active level becoming invalid
									// the only other way to fix it seems to be having the ReadFile
									// function allocate a keyboard dynamically, but then we lose
									// the pointer and it stays in memory forever
	return result;
}







BOOL Multikeys::Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName, IKeystrokeOutput ** out_action)
{
	
	// Look for correct device; return FALSE (= do not block) otherwise
	for (auto iterator = keyboards.begin(); iterator != keyboards.end(); iterator++)
	{
		if (wcscmp(iterator->deviceName.c_str(), deviceName) == 0)
		{
			// found the keyboard
			// ask it for what to do
			return iterator->evaluateKeystroke(
				keypressed->MakeCode,
				keypressed->VKey,
				(keypressed->Flags & RI_KEY_E0) == RI_KEY_E0,
				(keypressed->Flags & RI_KEY_E1) == RI_KEY_E1,
				(keypressed->Flags & RI_KEY_BREAK) == RI_KEY_BREAK,
				out_action);
			
		}
	}

	return FALSE;
}








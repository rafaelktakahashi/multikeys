#include "stdafx.h"

#include "Remaps.h"
#include "Keystrokes.h"














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
		testMethod(ptrVectorKeyboard);

		return TRUE;



	}

	
	static VOID testMethod(std::vector<KEYBOARD> * ptrVectorKeyboard)
	{
		ptrVectorKeyboard->clear();

		KEYBOARD keyboard = KEYBOARD();

		// The part of a device name in brackets identify the device itself,
		// while the characters before it change depending on port
		// Storing the entire device name will cause the check to fail for different ports
		keyboard.deviceName = L"{884b96c3-56ef-11d1-bc8c-00a0c91405dd}";
		keyboard.addModifier(VK_RSHIFT, FALSE);
		keyboard.addModifier(VK_LSHIFT, FALSE);



		Level level;
		level.setModifiers2();


		UINT characterArray[1];
		characterArray[0] = 0x2200;
		auto pointer1 = new UnicodeOutput(characterArray, 1, true);

		level.insertPair(0x02, pointer1);


		UINT anotherCharacterArray[1];
		anotherCharacterArray[0] = 0x2203;
		auto pointer2 = new UnicodeOutput(anotherCharacterArray, 1, true);

		level.insertPair(0x03, pointer2);

		DWORD macro1[4];
		macro1[0] = VK_LCONTROL;
		macro1[1] = 0x46;
		macro1[2] = 0x46 | 0x80000000;
		macro1[3] = VK_LCONTROL | 0x80000000;

		auto pointer3 = new MacroOutput(macro1, 4, true);

		level.insertPair(0x04, pointer3);



		UINT unicodeString[4];
		unicodeString[0] = 0x48;
		unicodeString[1] = 0x69;
		unicodeString[2] = 0x2e;
		unicodeString[3] = 0x1f642;		// prints "Hi." and a smiling emoji.
		auto pointer4 = new UnicodeOutput(unicodeString, 4, false);

		level.insertPair(0x05, pointer4);


		auto pointer5 =
			new ScriptOutput(L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe");

		level.insertPair(0x06, pointer5);


		DWORD macro2[4];
		macro2[0] = VK_LWIN;
		macro2[1] = VK_SPACE;
		macro2[2] = VK_SPACE | 0x80000000;
		macro2[3] = VK_LWIN | 0x80000000;
		auto pointer6 = new MacroOutput(macro2, 4, false);

		level.insertPair(0x07, pointer6);


		UINT unicodeString2[1];
		unicodeString2[0] = 0x1f468;
		auto pointer7 = new UnicodeOutput(unicodeString2, 1, true);
		level.insertPair(0x08, pointer7);

		UINT unicodeString3[1];
		unicodeString3[0] = 0x1F3FE;
		auto pointer8 = new UnicodeOutput(unicodeString3, 1, true);
		level.insertPair(0x09, pointer8);

		UINT unicodeString4[1];
		unicodeString4[0] = 0x1F3FD;
		auto pointer9 = new UnicodeOutput(unicodeString4, 1, true);
		level.insertPair(0x0a, pointer9);


		// Save this level
		keyboard.levels.push_back(level);

		// Add another level:


		level.setModifiers2(2, 2);		// LShift, RShift, either
		level.layout.clear();

		UINT unicodeString5[1];
		unicodeString5[0] = 0x2201;
		auto pointerShift1 = new UnicodeOutput(unicodeString5, 1, true);
		level.insertPair(0x02, pointerShift1);

		UINT unicodeString6[1];
		unicodeString6[0] = 0x2204;
		auto pointerShift2 = new UnicodeOutput(unicodeString6, 1, true);
		level.insertPair(0x03, pointerShift2);


		auto pointerShift5 = new ScriptOutput(
			L"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
			L"http://stackoverflow.com/");

		level.insertPair(0x06, pointerShift5);

		/*U+1F468 U+200D U+1F469 U+200D U+1F467 U+200D U+1F467 is family emoji*/
		UINT unicodePeople[7];
		unicodePeople[0] = 0x1f468;
		unicodePeople[1] = 0x200d;
		unicodePeople[2] = 0x1f469;
		unicodePeople[3] = 0x200d;
		unicodePeople[4] = 0x1f467;
		unicodePeople[5] = 0x200d;
		unicodePeople[6] = 0x1f467;
		auto pointerShift7 = new UnicodeOutput(unicodePeople, 7, true);
		level.insertPair(0x08, pointerShift7);

		UINT greekOxia[1];
		greekOxia[0] = 0x1ffd;
		UINT remapFrom[1];
		remapFrom[0] = 0x2200;		// the first remap
		UINT remapTo[1];		// For all
		remapTo[0] = 0x1fbb;	// A with oxia
		auto pointerRemapFrom = new UnicodeOutput(remapFrom, 1, true);
		auto pointerRemapTo = new UnicodeOutput(remapTo, 1, true);

		// another replacement:
		UINT remapFrom2[1];
		remapFrom2[0] = 0x1ffd;
		UINT remapTo2[1];
		remapTo2[0] = 0x1f10;
		auto pointerRemapFrom2 = new UnicodeOutput(remapFrom2, 1, true);
		auto pointerRemapTo2 = new UnicodeOutput(remapTo2, 1, true);

		UnicodeOutput* allRemapsFrom[2];
		allRemapsFrom[0] = pointerRemapFrom;
		allRemapsFrom[1] = pointerRemapFrom2;
		UnicodeOutput* allRemapsTo[2];
		allRemapsTo[0] = pointerRemapTo;
		allRemapsTo[1] = pointerRemapTo2;

		auto pointerShift8 = new DeadKeyOutput(greekOxia, 1, allRemapsFrom, allRemapsTo, 2);
		level.insertPair(0x09, pointerShift8);

		// Save shifted level
		keyboard.levels.push_back(level);

		// Refresh keyboard's state
		keyboard.resetModifierState();

		// Save keyboard
		ptrVectorKeyboard->push_back(keyboard);
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
	keyboards[0].resetModifierState();
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
		// checks if iterator->deviceName exists in deviceName (which includes port + device names)
		if (wcsstr(deviceName, iterator->deviceName.c_str()) != nullptr)
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








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
		ptrVectorKeyboard->clear();

		KEYBOARD keyboard = KEYBOARD();

		// The only part that seems to really matter is the string between curly brackets;
		// the characters before it seem to change depending on port
		// This might actually be useful for identifying keyboard depending either on device or USB port
		keyboard.deviceName = L"\\\\?\\HID#VID_0510&PID_0002#7&1b748abb&1&0000#{884b96c3-56ef-11d1-bc8c-00a0c91405dd}";
		keyboard.addModifier(VK_RSHIFT, FALSE);
		keyboard.addModifier(VK_LSHIFT, FALSE);
		


		Level level;
		level.setModifiers2();


		UINT characterArray[1];
		characterArray[0] = 0x1f3b5;
		auto pointer1 = new StringOutput(characterArray, 1);

		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x02, pointer1));

		
		DWORD macro0[2];
		macro0[0] = VK_SPACE;
		macro0[1] = VK_SPACE | 0x80000000;
		auto pointer2 = new MacroOutput(macro0, 4);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(0x03, pointer2));

		DWORD macro1[4];
		macro1[0] = VK_LCONTROL;
		macro1[1] = 0x46;
		macro1[2] = 0x46 | 0x80000000;
		macro1[3] = VK_LCONTROL | 0x80000000;

		auto pointer3 = new MacroOutput(macro1, 4);

		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x04, pointer3));



		UINT unicodeString[4];
		unicodeString[0] = 0x48;
		unicodeString[1] = 0x69;
		unicodeString[2] = 0x2e;
		unicodeString[3] = 0x1f642;		// prints "Hi." and a smiling emoji.
		auto pointer4 = new StringOutput(unicodeString, 4);

		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x05, pointer4));


		auto wideStringFilename = L"C:\\MultiKeys\\openAppTest.exe";
		auto pointer5 = new ScriptOutput(wideStringFilename);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>
			(0x06, pointer5));


		DWORD macro2[4];
		macro2[0] = VK_LWIN;
		macro2[1] = VK_SPACE;
		macro2[2] = VK_SPACE | 0x80000000;
		macro2[3] = VK_LWIN | 0x80000000;
		auto pointer6 = new MacroOutput(macro2, 4);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x07, pointer6));

		
		UINT unicodeString2[1];
		unicodeString2[0] = 0x1f468;
		auto pointer7 = new StringOutput(unicodeString2, 1);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x08, pointer7));

		
		// Save this level
		keyboard.levels.push_back(level);

		// Add another level:
		
		
		level.setModifiers2(2, 2);		// LShift, RShift, either
		level.layout.clear();

		UINT unicodeString3[1];
		unicodeString3[0] = 0x1f3b6;
		auto pointerShift1 = new StringOutput(unicodeString3, 1);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x02, pointerShift1));

		/*U+1F468 U+200D U+1F469 U+200D U+1F467 U+200D U+1F467 is family emoji*/
		UINT unicodePeople[7];
		unicodePeople[0] = 0x1f468;
		unicodePeople[1] = 0x200d;
		unicodePeople[2] = 0x1f469;
		unicodePeople[3] = 0x200d;
		unicodePeople[4] = 0x1f467;
		unicodePeople[5] = 0x200d;
		unicodePeople[6] = 0x1f467;
		auto pointerShift7 = new StringOutput(unicodePeople, 7);
		level.layout.insert(std::pair<DWORD, IKeystrokeOutput*>(0x08, pointerShift7));

		// Save shifted level
		keyboard.levels.push_back(level);
		
		// Refresh keyboard's state
		keyboard.resetModifierState();

		// Save keyboard
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








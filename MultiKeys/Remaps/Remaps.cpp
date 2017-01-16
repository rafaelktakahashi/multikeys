#include "Remaps.h"
#include "stdafx.h"

// Implementing the Remapper class prototyped in the Remaps header.

namespace Remaps
{
	BOOL EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName)
	{
		return TRUE;
	}

	BOOL LoadSettings()
	{
		return TRUE;
	}
}


// Structure to represent a keyboard and its remaps
struct KEYBOARD
{
	// Name of this device
	WCHAR * device_name;

	// Map between scancodes and their unicode remaps
	std::map<USHORT, WCHAR> remaps;

	KEYBOARD()
	{
		device_name = new WCHAR[128];
	}
};

// Keep it simple for now. Each key sends one character.
// We'll implement shortcuts on either side later, and maybe dead keys too.





// Good idea to use this name?
namespace MultiKeys
{
	class Remapper
	{
	private:

		// work variable
		WCHAR * wcharWork;

		// a vector of keyboards
		std::vector<KEYBOARD> keyboards;
		// to hold each keyboard

		BOOL Remapper::LoadSettings(std::string filename)		// parser
		{
			setlocale(LC_ALL, "");
			// in-file-stream: will only read
			std::ifstream file(filename.c_str());
			
			// hold each line here:
			std::string line;

			if (!file.is_open())
				return FALSE;	// oh, no!

			// read line by line:
			while (getline(file, line))		// guarantees read until newline, unline file >> line
			{
				// case one: device_name
				// read device name into new KEYBOARD structure
				// add its remaps one by one
				if (line == "device_name")
				{
					// next line is the keyboard name
					std::getline(file, line);
					KEYBOARD thisKeyboard;
					// copy keyboard name into newly created keyboard
					// (swprintf_s because destination is in unicode
					swprintf_s(thisKeyboard.device_name, sizeof(thisKeyboard.device_name), L"%s", (std::wstring( line.begin(), line.end() ).c_str() )  );
					// read the remaps one by one
					while (getline(file, line))
					{
						if (line == "remap")
						{
							// parse next line into a hexadecimal value
							getline(file, line);
							USHORT hexadecimal;
							scanf_s(line.c_str(), "%hX", &hexadecimal);		// <- h: short // X: hex
							// parse next line into a (unicode) character
							getline(file, line);
							ULONG codePoint;
							scanf_s(line.c_str(), "%lX", &codePoint);			// <- l: long // X: hex
							
							thisKeyboard.remaps.insert( std::pair<USHORT, WCHAR>(hexadecimal, (WCHAR)codePoint) );
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
				}
				else
				{
					// error: unexpected term
					return FALSE;
				}

				return TRUE;
			}
		}

		void Remapper::SimulateKeyStroke(WCHAR)
		{
			;	// idk
		}

	public:

		Remapper::Remapper(std::string filename)					// constructor
		{
			LoadSettings(filename);
		}

		BOOL Remapper::EvaluateKey(RAWKEYBOARD* keypressed, WCHAR* deviceName)
		{
			return TRUE;
		}

		BOOL Remapper::ReloadSettings(std::string filename)
		{
			LoadSettings(filename);
			return TRUE;
		}
	};
}
#pragma once

#include "stdafx.h"
#include "RemapperAPI.h"

// Move implementations into KeystrokeCommands.cpp later, when everything is already working.

namespace Multikeys
{

	// enum classes are strongly typed
	enum class KeystrokeOutputType
	{
		UnicodeCommand,
		MacroCommand,
		ScriptCommand,
		DeadKeyCommand,
		EmptyCommand
	};

	class BaseKeystrokeCommand : public IKeystrokeCommand
	{
	protected:
		INPUT unicodePrototype;
		INPUT VirtualKeyPrototypeDown;
		INPUT VirtualKeyPrototypeUp;

		BaseKeystrokeCommand()
		{
			// Initialize prototypes
			//
			// Prototypes exist to remove the derived classes' responsability to initialize their own INPUT
			// structure every time. This means every derived class contains these prototypes, even if they
			// don't need them; however, if everything goes correctly, the initialization of this class
			// should never occur at runtime.

			// When keyeventf_unicode is set, virtual key must be 0,
			// and the UTF-16 code value is put into wScan
			// Surrogate pairs require two consecutive inputs
			unicodePrototype.type = INPUT_KEYBOARD;
			unicodePrototype.ki.dwExtraInfo = 0;
			unicodePrototype.ki.dwFlags = KEYEVENTF_UNICODE;
			unicodePrototype.ki.time = 0;
			unicodePrototype.ki.wVk = 0;


			// Virtual keys are sent with the scancode e0 00 because our hook
			// will filter these out (to avoid responding to injected keys)
			VirtualKeyPrototypeDown.type = INPUT_KEYBOARD;
			VirtualKeyPrototypeDown.ki.dwExtraInfo = 0;
			VirtualKeyPrototypeDown.ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
			VirtualKeyPrototypeDown.ki.time = 0;
			VirtualKeyPrototypeDown.ki.wScan = 0;

			VirtualKeyPrototypeUp = VirtualKeyPrototypeDown;
			VirtualKeyPrototypeUp.ki.dwFlags |= KEYEVENTF_KEYUP;
		}

	public:

		virtual KeystrokeOutputType getType() const = 0;

		virtual bool execute(bool keyup, bool repeated) const override = 0;

		virtual ~BaseKeystrokeCommand() override = 0;
	};






	class MacroCommand : public BaseKeystrokeCommand
	{

	protected:

		INPUT * keystrokes;
		size_t inputCount;
		bool triggerOnRepeat;

	public:

		// STL constructor
		MacroCommand(std::vector<unsigned short> * const keypresses, bool triggerOnRepeat)
		{
			MacroCommand(keypresses->data(), keypresses->size(), triggerOnRepeat);
		}

		// unsigned short * keypressSequence - array of 16-bit values, each containing the virtual key code to be
		//		sent (1 byte value), and also the high bit (most significant) set in case of a keyup. Every keypress
		//		in this array will be sent in order of execution.
		// USHORT _inputCount - number of elements in keypressSequence
		// bool _triggerOnRepeat - true if this command should be triggered multiple times if user
		//		holds down the key
		MacroCommand(const unsigned short *const keypressSequence, const size_t _inputCount, const bool _triggerOnRepeat)
			: BaseKeystrokeCommand(), inputCount(_inputCount), triggerOnRepeat(_triggerOnRepeat)
		{
			if (keypressSequence == nullptr)
				return;

			keystrokes = new INPUT[_inputCount];

			bool keyup = 0;
			USHORT virtualKeyCode = 0;
			for (unsigned int i = 0; i < _inputCount; i++)
			{
				keyup = (keypressSequence[i] >> 15) & 1;
				virtualKeyCode = keypressSequence[i] & 0xff;
				keystrokes[i] = INPUT(VirtualKeyPrototypeDown);
				keystrokes[i].ki.wVk = virtualKeyCode;
				if (keyup) keystrokes[i].ki.dwFlags |= KEYEVENTF_KEYUP;
			}

		}	// end of constructor

		
		KeystrokeOutputType getType() const override
		{
			return KeystrokeOutputType::MacroCommand;
		}

		bool execute(bool keyup, bool repeated) const override
		{
			if (keyup)
				return TRUE;
			else if (!repeated || (repeated && triggerOnRepeat))
				return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
			else return TRUE;
		}

		~MacroCommand() override
		{
			delete[] keystrokes;
		}

	};

	class UnicodeCommand : public BaseKeystrokeCommand
	{

	protected:

		INPUT * keystrokes;
		size_t inputCount;
		bool triggerOnRepeat;

	public:

		// Constructor
		// The caller may let this container go out of scope.
		UnicodeCommand(const std::vector<unsigned int>& codepoints, const bool triggerOnRepeat)
		{
			inputCount = codepoints.size();
			for (size_t i = 0; i < codepoints.size(); i++)
			{
				if (codepoints[i] > 0xffff)
					inputCount++;		// <- correct the amount of inputs
			}
			keystrokes = new INPUT[inputCount];

			size_t currentIndex = 0;
			for (size_t i = 0; i < inputCount; i++)
			{
				if (codepoints[i] <= 0xffff)
				{
					// one UTF-16 code value, one simulated keypress
					keystrokes[currentIndex] = INPUT(unicodePrototype);
					keystrokes[currentIndex].ki.wScan = codepoints[i];
					currentIndex++;
					continue;
				}
				else
				{
					// one UTF-16 surrogate pair, two simulated keypresses
					WORD highSurrogate = 0xd800 + ((codepoints[i] - 0x10000) >> 10);
					WORD lowSurrogate = 0xdc00 + (codepoints[i] & 0x3ff);
					keystrokes[currentIndex] = INPUT(unicodePrototype);
					keystrokes[currentIndex + 1] = INPUT(unicodePrototype);
					keystrokes[currentIndex].ki.wScan = highSurrogate;
					keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
					currentIndex += 2;
					continue;
				}
			}
		}

		// UINT codepoints - array of UINTs, each containing a single Unicode code point
		//		identifying the character to be sent. All characters in this array will
		//		be sent in order on execution.
		//		This array may be deleted after this function is called.
		// UINT _inputCount - number of elements in codepoints
		// bool _triggerOnRepeat - true if this command should be triggered multiple times if user
		//		holds down the key
		UnicodeCommand(const UINT * const codepoints, const UINT _inputCount, const bool _triggerOnRepeat)
			: BaseKeystrokeCommand(), inputCount(_inputCount), triggerOnRepeat(_triggerOnRepeat)
		{
			if (codepoints == nullptr) return;

			for (unsigned int i = 0; i < _inputCount; i++)
			{
				if (codepoints[i] > 0xffff)
					inputCount++;			// <- actual amount of inputs
			}
			keystrokes = new INPUT[inputCount];

			unsigned int currentIndex = 0;
			for (unsigned int i = 0; i < _inputCount; i++)
			{
				if (codepoints[i] <= 0xffff)
				{
					// one UTF-16 code value, one simulated keypress
					keystrokes[currentIndex] = INPUT(unicodePrototype);
					keystrokes[currentIndex].ki.wScan = codepoints[i];
					currentIndex++;
					continue;		// next for
				}
				else
				{
					// UTF-16 surrogate pair, two simulated keypresses
					USHORT highSurrogate = 0xd800 + ((codepoints[i] - 0x10000) >> 10);
					USHORT lowSurrogate = 0xdc00 + (codepoints[i] & 0x3ff);
					keystrokes[currentIndex] = INPUT(unicodePrototype);
					keystrokes[currentIndex + 1] = INPUT(unicodePrototype);
					keystrokes[currentIndex].ki.wScan = highSurrogate;
					keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
					currentIndex += 2;
					continue;
				}
			}	// end for

		}	// end constructor

		KeystrokeOutputType getType() const override
		{
			return KeystrokeOutputType::UnicodeCommand;
		}

		bool execute(bool keyup, bool repeated) const override
		{
			if (keyup)	// Unicode keystrokes do not activate on release
				return TRUE;
			else if (!repeated || (repeated && triggerOnRepeat))
				return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
			else return TRUE;
		}

		// Comparing unicode keystrokes is important for a dead key.
		inline bool operator==(const UnicodeCommand& rhs) const
		{
			if (inputCount != rhs.inputCount) return false;
			// One by one, compare the keystroke inputs by codepoint.
			// We trust that the rest of each INPUT structure is the same
			for (size_t i = 0; i < inputCount; i++)
				if (this->keystrokes[i].ki.wScan != rhs.keystrokes[i].ki.wScan)
					return false;
			return true;
		}

		~UnicodeCommand() override
		{
			delete[] keystrokes;
		}
	};


	class ExecutableCommand : public BaseKeystrokeCommand
	{

	protected:

		std::wstring filename;
		std::wstring arguments;

	public:

		// std::wstring filename - The UTF-16 string containing a full path to the executable to be
		//		opened when this command is executed. This command does not close the executable.
		// std::wstring arguments - arguments to be passed to executable; multiple arguments must be
		//		separated by space
		// In practice, the file does not need to be an .exe executable specifically.
		ExecutableCommand(std::wstring filename, std::wstring arguments = std::wstring())
			: BaseKeystrokeCommand(), filename(filename), arguments(arguments)
		{}


		KeystrokeOutputType getType() const override
		{
			return KeystrokeOutputType::ScriptCommand;
		}

		bool execute(bool keyup, bool repeated) const override
		{
			if (repeated || keyup) return TRUE;

			// start process at filename

			HINSTANCE retVal =
				ShellExecute(NULL, L"open", filename.c_str(), arguments.c_str(), NULL, SW_SHOWNORMAL);
			if ((LONG)retVal <= 32)
				return FALSE;
			// TODO: handle errors


			return TRUE;
		}

		~ExecutableCommand() override
		{ }

	};

	// Dead keys are pressed before the key it modifies
	class DeadKeyCommand : public UnicodeCommand
	{
		// Inherits keystrokes, keystroke count and trigger on repeat from UnicodeCommand
		// Those fields describe this dead key as a standalone
	private:
		// 0 : no next command
		// 1 : unblocked command
		// 2 : remapped to unicode without valid substitute
		// 3 : remapped to non-unicode
		// 4 : remapped to unicode with a substitute
		// (another dead key will behave like a unicode)
		USHORT _nextCommandType;

		// Pointer to hold the command after this one.
		// NULL until input is received; remains null if the next input
		//		is not remapped (i. e. not blocked)
		mutable BaseKeystrokeCommand * _nextCommand;
		// mutable because this class's execute() is a const method that needs to modify it.

		// If this command is being remembered as active, then the next keystroke will cause
		// this method to be called.
		// command - The next command pressed on the same keyboard, null if the key pressed
		//		does not correspond to a remap.
		// vKey - Virtual key code of the keystroke that generated this. Only for checking
		//		special keys (esc, tab), and only present (non-zero) if command is null.
		void _setNextCommand(BaseKeystrokeCommand*const command, const USHORT vKey)
		{
			if (command == nullptr)
			{
				// means that the input is not supposed to be blocked
				// Might implement: Do not send this dead key's character if next key is either
				//		escape or tab.
				_nextCommand = nullptr;
				_nextCommandType = 1;
			}
			else
			{
				// first, make sure that the command is unicode (dead keys inherit from it):

				// All those that are not unicode are just placed as next command
				if (command->getType() != KeystrokeOutputType::UnicodeCommand
					&& command->getType() != KeystrokeOutputType::DeadKeyCommand)
				{
					_nextCommand = command;
					_nextCommandType = 3;
					return;
				}

				// It's a unicode (or dead key):
				// compare its input sequence with the replacement list
				// The map of replacements contains pointers, so using find() would actually
				//		search by pointer. That doesn't work.
				for (auto iterator = replacements.begin(); iterator != replacements.end(); iterator++)
				{
					// try to find the correct one (dereference pointers before comparing)
					// The actual UnicodeCommand objects are different, so comparing pointers won't work.
					if (*(iterator->first) == *(dynamic_cast<UnicodeCommand*>(command)))
					{
					// replace it
					_nextCommand = iterator->second;
					_nextCommandType = 4;
					return;
					}
				}
				// didn't find a suitable replacement
				_nextCommand = command;
				_nextCommandType = 2;

			}
		}
	public:


		// Replacements from Unicode codepoint sequence to Unicode outputs
		std::unordered_map<UnicodeCommand*, UnicodeCommand*> replacements;


		// STL constructor
		DeadKeyCommand(const std::vector<unsigned int>& const independentCodepoints,
			const std::unordered_map<UnicodeCommand*, UnicodeCommand*>& replacements)
			: UnicodeCommand(independentCodepoints, true),
			replacements(replacements) { }

		// UINT* independentCodepoints - the Unicode character for this dead key
		//								Array may be deleted after passing
		// UINT independentCodepointCount - the number of unicode characters for this dead key
		//								Array may be deleted after passing
		// UnicodeCommand** replacements_from - array of commands that consist valid sequences
		//								Array may be deleted after passing, but not each pointer
		// UnicodeCommand** replacements_to - array of commands that the codepoints map to
		//								Array may be deleted after passing, but not each pointer
		// UINT replacements_count - number of items in the previous arrays
		DeadKeyCommand(UINT*const independentCodepoints, UINT const independentCodepointsCount,
			UnicodeCommand**const replacements_from, UnicodeCommand**const replacements_to,
			UINT const replacements_count)
			: UnicodeCommand(independentCodepoints, independentCodepointsCount, true),
			_nextCommand(nullptr), _nextCommandType(0)
		{
			for (unsigned int i = 0; i < replacements_count; i++) {
				replacements[replacements_from[i]] = replacements_to[i];
			}
		}	// end of constructor



		// Call when next input is received. This object needs to know the next key, in order to decide
		//		what action to take when execute() is called on it.
		// IkeystrokeOutput* const command - pointer to the command to be executed right after this key;
		//		might be replaced if a match exists.
		void setNextCommand(BaseKeystrokeCommand*const command)
		{
			_setNextCommand(command, 0);
		}

		// Call when next input is received, before using this dead key
		// const USHORT vKey - the virtual key input by the user
		void setNextCommand(const USHORT vKey)
		{
			_setNextCommand(nullptr, vKey);
		}


		KeystrokeOutputType getType() const override
		{
			return KeystrokeOutputType::DeadKeyCommand;
		}


		bool execute(bool keyup, bool repeated = FALSE) const override
		{
			if (keyup) return TRUE;	// because unicode keyups do nothing

									// First, check for an edge case: If the next input is this one
									// That means infinite recursion because there's only one instance
									// of each given dead key (so every of its pointers will point there)
			if (_nextCommand == (BaseKeystrokeCommand*)this)
			{
				// just send this key twice
				SendInput(inputCount, keystrokes, sizeof(INPUT));
				SendInput(inputCount, keystrokes, sizeof(INPUT));
				// then clear the next pointer
				_nextCommand = nullptr;
			}

			// there is a replacement:
			if (_nextCommandType == 4)
			{
				return (_nextCommand->execute(keyup, repeated));
			}

			// Situation is normal; send this key, then the next
			SendInput(inputCount, keystrokes, sizeof(INPUT));
			if (_nextCommand != nullptr)
				return (_nextCommand->execute(keyup, repeated));
			else
				return TRUE;
		}

		~DeadKeyCommand() override
		{
			// is it necessary? --> delete _nextCommand;
			for (auto iterator = replacements.begin(); iterator != replacements.end(); iterator++)
			{
				delete iterator->first;
				delete iterator->second;
			}
		}

	};




	// Dummy output that performs no action when executed (good for modifier keys)
	class EmptyCommand : public BaseKeystrokeCommand
	{
	public:

		EmptyCommand() : BaseKeystrokeCommand() {}
		KeystrokeOutputType getType() const override { return KeystrokeOutputType::EmptyCommand; }
		bool execute(bool keyup, bool repeated = FALSE) const override { return TRUE; }
		~EmptyCommand() override {}
	};


}
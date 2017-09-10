#include "stdafx.h"

/*--Implementations of methods in KeystrokeCommands.h--*/
#include "KeystrokeCommands.h"

namespace Multikeys
{

	/*
	IKeystrokeCommand
	*/

	// destructors need an implementation, even if they are pure virtual.
	IKeystrokeCommand::~IKeystrokeCommand() { }

	/*
	BaseKeystrokeCommand
	*/
	
	BaseKeystrokeCommand::BaseKeystrokeCommand()
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

	// pure virtual destructor still needs implementation.
	BaseKeystrokeCommand::~BaseKeystrokeCommand() { }



	/*
	MacroCommand
	*/

	MacroCommand::MacroCommand(std::vector<unsigned short> * const keypresses, bool triggerOnRepeat)
	{
		MacroCommand(keypresses->data(), keypresses->size(), triggerOnRepeat);
	}

	MacroCommand::MacroCommand(const unsigned short *const keypressSequence, const size_t _inputCount, const bool _triggerOnRepeat)
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

	}

	KeystrokeOutputType MacroCommand::getType() const
	{
		return KeystrokeOutputType::MacroCommand;
	}

	bool MacroCommand::execute(bool keyup, bool repeated) const
	{
		if (keyup)
			return TRUE;
		else if (!repeated || (repeated && triggerOnRepeat))
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

	MacroCommand::~MacroCommand()
	{
		delete[] keystrokes;
	}



	/*
	UnicodeCommand
	*/

	UnicodeCommand::UnicodeCommand(const std::vector<unsigned int>& codepoints, const bool triggerOnRepeat)
	{
		inputCount = codepoints.size();
		for (size_t i = 0; i < codepoints.size(); i++)
		{
			if (codepoints[i] > 0xffff)
				inputCount++;		// <- correct the amount of inputs
		}
		keystrokes = new INPUT[inputCount];

		// Keep track of index inside the INPUT array:
		size_t currentIndex = 0;
		// Loop must use the amount of codepoints in vector, not the corrected amount of inputs:
		for (size_t i = 0; i < codepoints.size(); i++)
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
				uint16_t highSurrogate = 0xd800 + ((codepoints[i] - 0x10000) >> 10);
				uint16_t lowSurrogate = 0xdc00 + (codepoints[i] & 0x3ff);
				keystrokes[currentIndex] = INPUT(unicodePrototype);
				keystrokes[currentIndex + 1] = INPUT(unicodePrototype);
				keystrokes[currentIndex].ki.wScan = highSurrogate;
				keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
				currentIndex += 2;
				continue;
			}
		}
	}

	UnicodeCommand::UnicodeCommand(const UINT * const codepoints, const UINT _inputCount, const bool _triggerOnRepeat)
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
				uint16_t highSurrogate = 0xd800 + ((codepoints[i] - 0x10000) >> 10);
				uint16_t lowSurrogate = 0xdc00 + (codepoints[i] & 0x3ff);
				keystrokes[currentIndex] = INPUT(unicodePrototype);
				keystrokes[currentIndex + 1] = INPUT(unicodePrototype);
				keystrokes[currentIndex].ki.wScan = highSurrogate;
				keystrokes[currentIndex + 1].ki.wScan = lowSurrogate;
				currentIndex += 2;
				continue;
			}
		}	// end for

	}

	KeystrokeOutputType UnicodeCommand::getType() const
	{
		return KeystrokeOutputType::UnicodeCommand;
	}

	bool UnicodeCommand::execute(bool keyup, bool repeated) const
	{
		if (keyup)	// Unicode keystrokes do not activate on release
			return TRUE;
		else if (!repeated || (repeated && triggerOnRepeat))
			return (SendInput(inputCount, keystrokes, sizeof(INPUT)) == inputCount ? TRUE : FALSE);
		else return TRUE;
	}

	inline bool UnicodeCommand::operator==(const UnicodeCommand& rhs) const
	{
		if (inputCount != rhs.inputCount) return false;
		// One by one, compare the keystroke inputs by codepoint.
		// We trust that the rest of each INPUT structure is the same
		for (size_t i = 0; i < inputCount; i++)
			if (this->keystrokes[i].ki.wScan != rhs.keystrokes[i].ki.wScan)
				return false;
		return true;
	}

	UnicodeCommand::~UnicodeCommand()
	{
		delete[] keystrokes;
	}




	/*
	ExecutableCommand
	*/
	ExecutableCommand::ExecutableCommand(std::wstring filename, std::wstring arguments)
		: BaseKeystrokeCommand(), filename(filename), arguments(arguments)
	{}

	KeystrokeOutputType ExecutableCommand::getType() const
	{
		return KeystrokeOutputType::ScriptCommand;
	}

	bool ExecutableCommand::execute(bool keyup, bool repeated) const
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

	ExecutableCommand::~ExecutableCommand() { }



	/*
	DeadKeyCommand
	*/

	void DeadKeyCommand::_setNextCommand(BaseKeystrokeCommand*const command, const USHORT vKey)
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

	DeadKeyCommand::
		DeadKeyCommand(const std::vector<unsigned int>& independentCodepoints,
		const std::unordered_map<UnicodeCommand*, UnicodeCommand*>& replacements)
		: UnicodeCommand(independentCodepoints, true),
		replacements(replacements) { }

	DeadKeyCommand::
		DeadKeyCommand(UINT*const independentCodepoints, UINT const independentCodepointsCount,
		UnicodeCommand**const replacements_from, UnicodeCommand**const replacements_to,
		UINT const replacements_count)
		: UnicodeCommand(independentCodepoints, independentCodepointsCount, true),
		_nextCommand(nullptr), _nextCommandType(0)
	{
		for (unsigned int i = 0; i < replacements_count; i++) {
			replacements[replacements_from[i]] = replacements_to[i];
		}
	}

	void DeadKeyCommand::setNextCommand(BaseKeystrokeCommand*const command)
	{
		_setNextCommand(command, 0);
	}
	void DeadKeyCommand::setNextCommand(const USHORT vKey)
	{
		_setNextCommand(nullptr, vKey);
	}

	KeystrokeOutputType DeadKeyCommand::getType() const
	{
		return KeystrokeOutputType::DeadKeyCommand;
	}

	bool DeadKeyCommand::execute(bool keyup, bool repeated) const
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

	DeadKeyCommand::~DeadKeyCommand()
	{
		// is it necessary? --> delete _nextCommand;
		for (auto iterator = replacements.begin(); iterator != replacements.end(); iterator++)
		{
			delete iterator->first;
			delete iterator->second;
		}
	}



}
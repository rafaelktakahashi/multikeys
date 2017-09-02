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

	/*
	BaseKeystrokeCommand - Base class for all commands, for internal use. Inherits from
	IKeystrokeCommand, which is visible outside this library.
	*/
	class BaseKeystrokeCommand : public IKeystrokeCommand
	{
	protected:
		INPUT unicodePrototype;
		INPUT VirtualKeyPrototypeDown;
		INPUT VirtualKeyPrototypeUp;

		BaseKeystrokeCommand();

	public:

		virtual KeystrokeOutputType getType() const = 0;

		virtual bool execute(bool keyup, bool repeated) const override = 0;

		virtual ~BaseKeystrokeCommand() override = 0;
	};






	class MacroCommand : public BaseKeystrokeCommand
	{

	private:

		INPUT * keystrokes;
		size_t inputCount;
		bool triggerOnRepeat;

	public:

		// STL constructor
		MacroCommand(std::vector<unsigned short> * const keypresses, bool triggerOnRepeat);

		// unsigned short * keypressSequence - array of 16-bit values, each containing the virtual key code to be
		//		sent (1 byte value), and also the high bit (most significant) set in case of a keyup. Every keypress
		//		in this array will be sent in order of execution.
		// USHORT _inputCount - number of elements in keypressSequence
		// bool _triggerOnRepeat - true if this command should be triggered multiple times if user
		//		holds down the key
		MacroCommand(const unsigned short *const keypressSequence, const size_t _inputCount, const bool _triggerOnRepeat);

		KeystrokeOutputType getType() const override;

		bool execute(bool keyup, bool repeated) const override;

		~MacroCommand() override;

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
		UnicodeCommand(const std::vector<unsigned int>& codepoints, const bool triggerOnRepeat);

		// UINT codepoints - array of UINTs, each containing a single Unicode code point
		//		identifying the character to be sent. All characters in this array will
		//		be sent in order on execution.
		//		This array may be deleted after this function is called.
		// UINT _inputCount - number of elements in codepoints
		// bool _triggerOnRepeat - true if this command should be triggered multiple times if user
		//		holds down the key
		UnicodeCommand(const UINT * const codepoints, const UINT _inputCount, const bool _triggerOnRepeat);

		KeystrokeOutputType getType() const override;

		bool execute(bool keyup, bool repeated) const override;

		// Comparing unicode keystrokes is important for a dead key.
		inline bool operator==(const UnicodeCommand& rhs) const;

		~UnicodeCommand() override;
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
		ExecutableCommand(std::wstring filename, std::wstring arguments = std::wstring());

		KeystrokeOutputType getType() const override;

		bool execute(bool keyup, bool repeated) const override;

		~ExecutableCommand() override;

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
		void _setNextCommand(BaseKeystrokeCommand*const command, const USHORT vKey);

	public:


		// Replacements from Unicode codepoint sequence to Unicode outputs
		std::unordered_map<UnicodeCommand*, UnicodeCommand*> replacements;


		// STL constructor
		DeadKeyCommand(const std::vector<unsigned int>& independentCodepoints,
			const std::unordered_map<UnicodeCommand*, UnicodeCommand*>& replacements);

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
			UINT const replacements_count);



		// Call when next input is received. This object needs to know the next key, in order to decide
		//		what action to take when execute() is called on it.
		// IkeystrokeOutput* const command - pointer to the command to be executed right after this key;
		//		might be replaced if a match exists.
		void setNextCommand(BaseKeystrokeCommand*const command);

		// Call when next input is received, before using this dead key
		// const USHORT vKey - the virtual key input by the user
		void setNextCommand(const USHORT vKey);


		KeystrokeOutputType getType() const override;


		bool execute(bool keyup, bool repeated = FALSE) const override;

		~DeadKeyCommand() override;

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
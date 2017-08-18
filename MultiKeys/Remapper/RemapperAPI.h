#pragma once

#include "stdafx.h"

namespace Multikeys
{
	// Class that represents a sequence of keystrokes or character
	// or an executable file.
	typedef class IKeystrokeCommand
	{
	public:

		// Simulate the keytroke sequence that this object represents
		virtual BOOL execute(BOOL keyup, BOOL repeated) const = 0;

		// Virtual destructor
		virtual ~IKeystrokeCommand() = 0;

	} *PKeystrokeCommand;


	// Class that holds an internal model of the user's remapped keyboards;
	// can be queried for a remapped command of a given keypress
	typedef class IRemapper
	{
	public:

		// Opens configuration file at filename and loads
		// its remaps it into this instance. Returns FALSE if any
		// error was encountered.
		virtual bool loadSettings(const std::wstring xmlFilename) = 0;

		// Evaluates a user keypress according to loaded remaps.
		// -- Parameters --
		// RAWKEYBOARD* keypressed - information about the user keypress
		// WCHAR* deviceName - full name of the device that generated the input
		// OUT IKeystrokeCommand** out_action - command to be executed instead
		//			of the user input, in case it should be blocked.
		// -- Return value --
		// TRUE - User input should be blocked, and out_action should be executed.
		// FALSE - Do not block user input and do not execute out_action.
		virtual bool evaluateKey(
			RAWKEYBOARD* const keypressed,
			WCHAR* const deviceName,
			OUT PKeystrokeCommand* const out_action) const = 0;

		virtual ~IRemapper() = 0;

	} *PRemapper;


	// Places a new instance of a Remapper class into pointer
	//	pointed to by instance.
	void Create(OUT PRemapper* instance)
	{
		*instance = new Remapper();
	}

	// Deletes the object located at the pointer pointed to by
	//	instance; then, that pointer becomes null.
	void Destroy(PRemapper* instance)
	{
		delete (*instance);
		*instance = nullptr;
	}
	// TODO: Move implementation of these factory methods into Remappper.h
}
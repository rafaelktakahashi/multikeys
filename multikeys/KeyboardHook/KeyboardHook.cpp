

#include "stdafx.h"
#include <stdio.h>
#include "KeyboardHook.h"

// Creating the shared data segment (between the main program and the DLL)
// The specific name for the shared segment is not important.
#pragma data_seg(".SHARED")
// Windows message for communication between main executable and DLL module
UINT const WM_HOOK = WM_APP + 1;
// WHND of the main executable (managing application)
HWND hwndServer = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARED,RWS")
// Pragma comment causes the linker to have the command line switch shown added to the link step

// There exists the function RegisterWindowsMessage to get a unique message identifier
// But we should only use it when more than one application processes the same message
// It's okay to choose some number (we chose WM_APP + 1) for private messages

HINSTANCE instanceHandle;
HHOOK hookHandle;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		instanceHandle = hModule;
		hookHandle = NULL;
		break;
	default:
		break;
	}
	return TRUE;
}

// Keyboard Hook procedure (this syntax is necessary for all hook procedures)
// This hook can block keys, Raw Input cannot.
// int code - either 0 (process), smaller than 0 (do not process), or 3 (someone called PeekMessage)
// WPARAM wParam - virtual key code of the key that generated the message
// LPARAM lParam - repeat count, scan code, extended-key flag, context code, previous state and transition state
static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{

	// Code smaller than zero (HC_ACTION) means "forward the message without further processing"
	if (code != HC_ACTION)
	{	// If another application calls PeekMessage, we will receive an extra input message
		// carrying the code 3 (HC_NOREMOVE). We don't respond to these.
		return CallNextHookEx(hookHandle, code, wParam, lParam);
	}

	
	// A virtual key code of e7 is used to pass unicode characters as if they were keystrokes
	// In that case, the keystroke must have originated from our own fake SendInput keypress,
	// and will not correspond to a raw input message. Thus, we ignore any message of vKey e7
	if (wParam == 0xe7)
		return CallNextHookEx(hookHandle, code, wParam, lParam);


	// Injected virtual-key presses are identified by us by a scancode of e0 00
	if (((lParam >> 16) & 0xff) == 0		// Refer to keystroke message flags
		&& ((lParam >> 24) & 1) == 1)
	{										// did you know that equality (==, !=) takes precedence over bitwise operators?
		return CallNextHookEx(hookHandle, code, wParam, lParam);
	}
	

	// Report the event to the main window.
	// Return value of 1 means block the input,
	// return value of 0 means pass it along the hook chain
	// (sends a nonqueued message to hwndServer)
	if (SendMessage(hwndServer, WM_HOOK, wParam, lParam))		// <-WndProc in main exe
	{
		// Also, we use SendMessage instead of PostMessage (queue) because we want to wait for the decision.
		return 1;		// don't pass it along. This will cancel the keystroke.
	}

	// must call next procedure in the hook chain
	return CallNextHookEx(hookHandle, code, wParam, lParam);
}

// returns TRUE if successful
BOOL InstallHook(HWND hwndParent)
{
	if (hwndServer != NULL)
	{
		// Already hooked
		return FALSE;
	}

	// Register global keyboard hook (at the beginning of the hook chain) using our keyboard procedure
	// params: idHook (type of hook to install), lpfn (pointer to hook procedure),
	// hMod (handle to the DLL), dwThreadId (thread to associate the hook procedure, 0 means all)
	hookHandle = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, instanceHandle, 0);
	if (hookHandle == NULL)		// security check
		return FALSE;
	hwndServer = hwndParent;
	return TRUE;
}


// returns true if successful
BOOL UninstallHook()
{
	if (hookHandle == NULL)
	{
		return TRUE;
	}
	// If unhook attempt fails, check whether it is because of invalid handle (in that case continue)
	if (!UnhookWindowsHookEx(hookHandle))
	{
		DWORD error = GetLastError();
		if (error != ERROR_INVALID_HOOK_HANDLE)
		{
			return FALSE;
		}
	}
	hwndServer = NULL;
	hookHandle = NULL;
	return TRUE;
}



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
// int code determines the action to perform; depends on the type of hook
static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (code < 0)
	{
		return CallNextHookEx(hookHandle, code, wParam, lParam);
	}

	// Report the event to the main window. Return value of 1 means block the input,
	// return value of 0 means pass it along the hook chain
	// (sends a nonqueued message to hwndServer)
	if (SendMessage(hwndServer, WM_HOOK, wParam, lParam))		// <-WndProc in main exe
	{
		// Also, we use SendMessage instead of PostMessage (queue) because we want to wait for the decision.
		return 1;
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
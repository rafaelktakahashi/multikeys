// Based on the HookingRawInput demo by Vít Blecha (sethest@gmail.com), 2014
// Written by Rafael Takahashi (rafael.k.takahashi@gmail.com), 2017.

// This is the keyboard hook for the program.
// A global hook procedure can be called in the context of any application in the same
// desktop as the calling thread, so the procedure must be in a separate DLL module.

#pragma once

// Windows types
#include <windef.h>

// did not include (or create) resource.h


#define HOOKINGRAWINPUTDLL_API __declspec(dllexport)

// Exporting prototypes:
HOOKINGRAWINPUTDLL_API BOOL InstallHook(HWND hwndParent);

HOOKINGRAWINPUTDLL_API BOOL UninstallHook();
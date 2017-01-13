// MultiKeys.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MultiKeys.h"
#include "..\HookRawInput\KeyboardHook.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


//HWND of main executable
HWND mainHwnd;
// Windows message for communication between main executable and DLL module
UINT const WM_HOOK = WM_APP + 1;
// How long should hook processing wait for the matching Raw Input message (in ms)
DWORD maxWaitingTime = 100;
// Device name of keyboard (fix this)
WCHAR* const numericKeyboardDeviceName = L"\\\\?\\HIDjjkjkijeksdf";

// Buffer for the decisions whether to block the input with Hook
std::deque<DecisionRecord> decisionBuffer;



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MULTIKEYS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTIKEYS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTIKEYS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MULTIKEYS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }



   // Save the HWND
   mainHwnd = hWnd;



   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);


   
   // Register for receiving Raw Input for keyboards
   RAWINPUTDEVICE rawInputDevice[1];
   rawInputDevice[0].usUsagePage = 1;		// usage page = 1 and usage = 6 is for keyboards
   rawInputDevice[0].usUsage = 6;
   rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;
   rawInputDevice[0].hwndTarget = hWnd;
   RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));

   // Setup the keyboard hook (from the dll)
   InstallHook(hWnd);
   // This call to intall the hook is accesible from here because we included the header from the DLL project
   // the actual DLL is also included as a reference in this project.
   // The hook needs to be executed from a separate dll because it's a global hook.


   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
	// Raw Input Message:
	case WM_INPUT:
	{	// brackets for locality
		UINT bufferSize;

		// Prepare buffer for the data
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));
		LPBYTE dataBuffer = new BYTE[bufferSize];
		// Load data into the buffer
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, dataBuffer, &bufferSize, sizeof(RAWINPUTHEADER));

		RAWINPUT* raw = (RAWINPUT*)dataBuffer;

		// Get the virtual key code of the key and report it
		USHORT virtualKeyCode = raw->data.keyboard.VKey;
		USHORT keyPressed = raw->data.keyboard.Flags & RI_KEY_BREAK ? 0 : 1;
		WCHAR text[128];
		swprintf_s(text, 128, L"Raw Input: %X (%d)\n", virtualKeyCode, keyPressed);		// <- THIS GOES TO DEBUG WINDOW
		OutputDebugString(text);

		// Prepare string buffer for the device name
		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, NULL, &bufferSize);
		WCHAR* stringBuffer = new WCHAR[bufferSize];

		// Load the device name into the buffer
		GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, stringBuffer, &bufferSize);
		// Now stringBuffer contains the device that sent the signal


		// EXAMPLE CHECKING - IMPLEMENT LOGIC HERE
		// Check whether the key struck was a "7" of a numeric keyboard, and remember the decision whether to block the input
		BOOL DoBlock = (virtualKeyCode == 0x67 && wcscmp(stringBuffer, numericKeyboardDeviceName) == 0) ? TRUE : FALSE;
		decisionBuffer.push_back(DecisionRecord(virtualKeyCode, DoBlock));

		// cleanup
		delete[] stringBuffer;
		delete[] dataBuffer;
		return 0;	// exit Wndproc
	}
		// end of case WM_INPUT


	// Message from Hooking DLL
	// It means we need to look for a corresponding Raw Input message that should have arrived before
	// That message is the one that can tell us whether or not to block the key,
	// and this message is the one that can block the key.
	// (or maybe it didn't arrive yet)
	case WM_HOOK:
	{
		USHORT virtualKeyCode = (USHORT)wParam;
		USHORT keyPressed = lParam & 0x80000000 ? 0 : 1;

		WCHAR text[128];
		swprintf_s(text, 128, L"Hook: %X (%d)\n", virtualKeyCode, keyPressed);
		OutputDebugString(text);	// <- sends to debug window

		// Check the Raw Input buffer to see if this Hook message is supposed to be blocked; WdnProc returns 1 if it is
		BOOL blockThisHook = FALSE;
		BOOL recordFound = FALSE;
		if (!decisionBuffer.empty())
		{
			int index = 0;
			// Search the buffer for the matching record (it should exist)
			for (std::deque<DecisionRecord>::iterator iterator = decisionBuffer.begin();
				iterator != decisionBuffer.end();
				iterator++, index++)
			{
				if (iterator->virtualKeyCode == virtualKeyCode)		// match!
				{
					blockThisHook = iterator->decision;	// this was decided somewhere else
					recordFound = TRUE;		// set the flag
					// Then, remove this and all preceding messages from the buffer
					for (int i = 0; i <= index; i++)		// <- up until and including this one
						decisionBuffer.pop_front();

					// We found the item, so we break the for loop to stop looking:
					break;

				} // end if match

			} // end for search

		} // end if buffer.empty

		// Wait for the matching Raw Input message if the decision buffer was empty or the matching record wasn't there
		DWORD currentTime, startTime;
		startTime = GetTickCount();
		while (!recordFound)					// This looks like a lot of work. Aren't we doing too much?
		{
			MSG rawMessage;
			while (!PeekMessage(&rawMessage, mainHwnd, WM_INPUT, WM_INPUT, PM_REMOVE))		// this does not remove the message
			{
				// Test for the maxWaitingTime
				currentTime = GetTickCount();
				// If current time is less than start, the time rolled over to 0
				if ((currentTime < startTime ? ULONG_MAX - startTime + currentTime : currentTime - startTime) > maxWaitingTime)
				{
					// Ignore the Hook message if it exceeded the limit
					WCHAR text[128];
					swprintf_s(text, 128, L"Hook timed out: %X (%d)\n", virtualKeyCode, keyPressed);
					OutputDebugString(text);
					return 0;
				}

				// Take a nap until checking again.
				Sleep(20);			// Take this out if it causes problems.

			}

			// The Raw Input message has arrived; decide whether to block the input
			UINT bufferSize;

			// Prepare buffer for the data
			// GetRawInputData((HRAWINPUT)rawMessage.lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER));	// <- is this needed?
			LPBYTE dataBuffer = new BYTE[bufferSize];
			// Load data into the buffer
			GetRawInputData((HRAWINPUT)rawMessage.lParam, RID_INPUT, dataBuffer, &bufferSize, sizeof(RAWINPUTHEADER));

			RAWINPUT* raw = (RAWINPUT*)dataBuffer;			// why a pointer?

			// Get the virtual key code of the key and report it
			USHORT rawVirtualKeyCode = raw->data.keyboard.VKey;
			USHORT rawKeyPressed = raw->data.keyboard.Flags & RI_KEY_BREAK ? 0 : 1;
			WCHAR text[128];
			swprintf_s(text, 128, L"Raw Input waiting: %X (%d)\n", rawVirtualKeyCode, rawKeyPressed);
			OutputDebugString(text);

			// Prepare string buffer for the device name
			// GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, NULL, &bufferSize);	// <- not needed
			WCHAR* stringBuffer = new WCHAR[bufferSize];

			// Load the device name into the buffer
			GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, stringBuffer, &bufferSize);

			// If the raw input message doesn't match the hook, push it into the buffer and continue waiting
			if (virtualKeyCode != rawVirtualKeyCode)
			{
				// Hey, why are doing this here?
				if (rawVirtualKeyCode == 0x67)
					decisionBuffer.push_back(DecisionRecord(rawVirtualKeyCode, TRUE));
				else
					decisionBuffer.push_back(DecisionRecord(rawVirtualKeyCode, FALSE));
			}
			else
			{
				// Correct raw input message
				recordFound = TRUE;
				// Yet another example checking
				if (rawVirtualKeyCode == 0x67)
					blockThisHook = TRUE;
				else blockThisHook = FALSE;
			}

			delete[] stringBuffer;
			delete[] dataBuffer;
		}

		// Apply the decision
		if (blockThisHook)
		{
			swprintf_s(text, 128, L"Keyboard event: %X (%d) is being blocked!\n", virtualKeyCode, keyPressed);
			OutputDebugString(text);
			return 1;		// exit WndProc
		}
		return 0;			// also exitEndProc

	}	// end of case WM_HOOK


    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:		// Give it to someone else.
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

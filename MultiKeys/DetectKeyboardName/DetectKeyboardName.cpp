// DetectKeyboardName.cpp : Defines the entry point for the application.
// Rafael Kenji Takahashi - rafael.k.takahashi@gmail.com

/*

This detects a keyboard input through the Raw Input API, prints the name of
that detected keyboard to standard output, then exits immediately.
Since this is a Win32 (graphical) application (which it must be, in order
to receive Raw Input messages), the stdout is never shown (not even when
executing from the command prompt). When executing this application, you
must redirect the standard output; the following command will create a text
file containing the name of the keyboard with which the command was input.
	DetectKeyboardName.exe > output.txt

*/

#include "stdafx.h"
#include "DetectKeyboardName.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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
	LoadStringW(hInstance, IDC_DETECTKEYBOARDNAME, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DETECTKEYBOARDNAME));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		/*if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{*/
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		/*}*/
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
	WNDCLASSEXW wcex = { };

	wcex.cbSize = sizeof(WNDCLASSEX);

	// wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	// wcex.cbClsExtra     = 0;
	// wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	// wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DETECTKEYBOARDNAME));
	// wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	// wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	// wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DETECTKEYBOARDNAME);
	wcex.lpszClassName  = szWindowClass;
	// wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   // ShowWindow(hWnd, nCmdShow);
   // UpdateWindow(hWnd);

   // Register for receiving  Raw Input for keyboards
   RAWINPUTDEVICE rawInputDevice[1];
   rawInputDevice[0].usUsagePage = 1;		// usage page = 1 is generic and usage = 6 is for keyboards
   rawInputDevice[0].usUsage = 6;				// (2 is mouse, 4 is joystick, 6 is keyboard, there are others)
   rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;		// Receive input even if the registered window is in the background
   rawInputDevice[0].hwndTarget = hWnd;				// Handle to the target window (NULL would make it follow kb focus)
   RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));

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
	{
		UINT bufferSize = 0;	// will be initialized by call to GetRawInputData

		// Pointer to a Raw Input structure
		RAWINPUT * raw;


		// Get data from the raw input structure
		// Parameters:
		// 1. HRAWINPUT - a handle to the raw input structure (in this case stored in the long param of the message)
		// 2. UINT - a flag containing what to return, either RID_HEADER (header) or RID_INPUT (raw data)
		// 3. LPVOID, out param - a pointer to the data that comes from the RAW INPUT structure
		//		If this is null, the next parameter will contain the required size of the buffer
		// 4. PUINT, in or out param - size of the data, or required size if previous param is NULL
		// 5. UINT - size in bytes of the header
		GetRawInputData(
			(HRAWINPUT)lParam, RID_INPUT, NULL, &bufferSize, sizeof(RAWINPUTHEADER)
		);

		// Create a buffer for the keyboard Raw Input struct
		LPBYTE rawKeyboardBuffer = new BYTE[bufferSize];

		// Load data into buffer
		GetRawInputData(
			(HRAWINPUT)lParam,
			RID_INPUT,
			rawKeyboardBuffer,
			&bufferSize,
			sizeof(RAWINPUTHEADER)
		);

		// Cast the contents of the buffer into our rawinput pointer
		raw = (RAWINPUT*)rawKeyboardBuffer;

		// Get the device name
		GetRawInputDeviceInfo(
			raw->header.hDevice,
			RIDI_DEVICENAME,
			NULL,
			&bufferSize
		);

		// Create a buffer large enough for the name
		WCHAR * keyboardNameBuffer = new WCHAR[bufferSize];

		// Load the device name into the buffer
		GetRawInputDeviceInfo(
			raw->header.hDevice,
			RIDI_DEVICENAME,
			keyboardNameBuffer,
			&bufferSize
		);

		// Print to standard output
		// (stdout is never visible; see note at the top of this file)
		wprintf_s(L"%s", keyboardNameBuffer);
		OutputDebugString(keyboardNameBuffer);
		// Close the application
		PostQuitMessage(0);
		break;
	}
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
	default:
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

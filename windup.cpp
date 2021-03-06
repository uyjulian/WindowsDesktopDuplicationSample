// windup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "windup.h"
#include "WinDesktopDup.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                         // current instance
WCHAR     szTitle[MAX_LOADSTRING];       // The title bar text
WCHAR     szWindowClass[MAX_LOADSTRING]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM    MyRegisterClass(HINSTANCE hInstance);
HWND    InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

WinDesktopDup dup;

void SetupDpiAwareness() {
	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
		tsf::print("SetProcessDpiAwarenessContext failed\n");
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int    nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	SetupDpiAwareness();

	auto err = dup.Initialize();
	//err               = dup.Capture();

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDUP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	HWND wnd = InitInstance(hInstance, nCmdShow);
	if (!wnd)
		return FALSE;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDUP));

	SetTimer(wnd, 1, 15, nullptr);

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
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
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDUP));
	wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_WINDUP);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
HWND InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	                          CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return nullptr;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_COMMAND: {
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	} break;
	case WM_TIMER:
		if (dup.CaptureNext())
			InvalidateRect(hWnd, nullptr, false);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC         hdc = BeginPaint(hWnd, &ps);
		BITMAPINFO  inf;
		memset(&inf, 0, sizeof(inf));
		inf.bmiHeader.biSize        = sizeof(inf.bmiHeader);
		inf.bmiHeader.biWidth       = dup.Latest.Width;
		inf.bmiHeader.biHeight      = -dup.Latest.Height;
		inf.bmiHeader.biPlanes      = 1;
		inf.bmiHeader.biBitCount    = 32;
		inf.bmiHeader.biCompression = BI_RGB;
		void*   bits                = nullptr;
		HDC     srcDC               = CreateCompatibleDC(hdc);
		HBITMAP dib                 = CreateDIBSection(hdc, &inf, 0, &bits, nullptr, 0);
		memcpy(bits, dup.Latest.Buf.data(), dup.Latest.Width * dup.Latest.Height * 4);
		SelectObject(srcDC, dib);
		BitBlt(hdc, 0, 0, dup.Latest.Width, dup.Latest.Height, srcDC, 0, 0, SRCCOPY);
		DeleteObject(dib);
		DeleteObject(srcDC);
		EndPaint(hWnd, &ps);
	} break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR) TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR) TRUE;
		}
		break;
	}
	return (INT_PTR) FALSE;
}

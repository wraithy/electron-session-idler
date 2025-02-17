#include <windows.h>
#include <iostream>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void LockAndSuspend(HWND hStatusLabel);
void UnlockAndResume(HWND hStatusLabel);

const wchar_t* kPowerMonitorWindowClass = L"Electron_PowerMonitorHostWindow";

void LockAndSuspend(HWND hStatusLabel) {
	HWND hWindow = NULL;

	do {
		hWindow = FindWindowEx(NULL, hWindow, kPowerMonitorWindowClass, NULL);
		if (hWindow) {
			DWORD sessionID = 0;
			ProcessIdToSessionId(GetCurrentProcessId(), &sessionID);

			SendMessage(hWindow, WM_WTSSESSION_CHANGE, WTS_SESSION_LOCK, sessionID);
			SendMessage(hWindow, WM_POWERBROADCAST, PBT_APMSUSPEND, NULL);
		}
	} while (hWindow != NULL);

	SetWindowText(hStatusLabel, L"Status: Enabled");
}

void UnlockAndResume(HWND hStatusLabel) {
	HWND hWindow = NULL;

	do {
		hWindow = FindWindowEx(NULL, hWindow, kPowerMonitorWindowClass, NULL);
		if (hWindow) {
			DWORD sessionID = 0;
			ProcessIdToSessionId(GetCurrentProcessId(), &sessionID);

			SendMessage(hWindow, WM_POWERBROADCAST, PBT_APMRESUMEAUTOMATIC, NULL);
			SendMessage(hWindow, WM_WTSSESSION_CHANGE, WTS_SESSION_UNLOCK, sessionID);
		}
	} while (hWindow != NULL);

	SetWindowText(hStatusLabel, L"Status: Disabled");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"NotificationForcerWindow";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0, CLASS_NAME, L"[ESI] Electron Session Idler", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 300, 180, NULL, NULL, hInstance, NULL);

	if (!hwnd) return 0;

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND hEnableButton, hDisableButton, hStatusLabel;
	static HFONT hFont;

	switch (uMsg) {
	case WM_CREATE:
		hFont = CreateFont(
			16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Courier New");

		hEnableButton = CreateWindow(
			L"BUTTON", L"Enable", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			50, 30, 80, 30, hwnd, (HMENU)1, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		SendMessage(hEnableButton, WM_SETFONT, (WPARAM)hFont, TRUE);

		hDisableButton = CreateWindow(
			L"BUTTON", L"Disable", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			150, 30, 80, 30, hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		SendMessage(hDisableButton, WM_SETFONT, (WPARAM)hFont, TRUE);

		hStatusLabel = CreateWindow(
			L"STATIC", L"Status: Disabled", WS_VISIBLE | WS_CHILD | SS_CENTER,
			50, 90, 180, 20, hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
		SendMessage(hStatusLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == 1) {
			LockAndSuspend(hStatusLabel);
		}
		else if (LOWORD(wParam) == 2) {
			UnlockAndResume(hStatusLabel);
		}
		break;

	case WM_DESTROY:
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

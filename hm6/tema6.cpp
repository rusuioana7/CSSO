#include <Windows.h>
#include <Commdlg.h>
#include <string>
#include <vector>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OpenImageDialog(HWND hwnd);
void DisplayHeaderInfo(HWND hwnd);

HWND g_hButton;
HWND g_hImageBox;
HWND g_hGrayscaleButton;
HWND g_hInversareByteButton;
HWND g_hPathTextbox;
HWND g_hPathTextboxInv;
HWND g_hComputerInfoLabel;
HWND g_hFeatureBox;
HWND g_hHeaderInfoBox;
HWND g_hHeaderInfoLabel;
HWND g_hTestComboBox;
HWND g_hRunTimesListBox;
wchar_t g_szFileName[MAX_PATH] = L""; 

std::vector<std::wstring> g_testNames = { L"Test 1", L"Test 2", L"Test 3" };

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	WNDCLASSW wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"MyWindowClass";
	wc.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(
		0,
		L"MyWindowClass",
		L"My Window",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1000, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	HWND hBlueBox = CreateWindowW(
		L"STATIC",
		L"",
		WS_VISIBLE | WS_CHILD | SS_SUNKEN,
		10, 50, 300, 300,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hButton = CreateWindowW(
		L"BUTTON",
		L"Open Image",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10, 10, 100, 30,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hImageBox = CreateWindowW(
		L"STATIC",
		L"",
		WS_VISIBLE | WS_CHILD | SS_BITMAP,
		10, 10, 280, 280,
		hBlueBox,
		NULL,
		hInstance,
		NULL
	);

	g_hGrayscaleButton = CreateWindowW(
		L"BUTTON",
		L"Grayscale",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10, 400, 100, 30,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hInversareByteButton = CreateWindowW(
		L"BUTTON",
		L"Inversare Byte",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10, 480, 120, 30,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hPathTextbox = CreateWindowW(
		L"EDIT",
		L"",
		WS_VISIBLE | WS_CHILD | ES_READONLY | WS_BORDER,
		10, 440, 200, 20,
		hwnd,
		NULL,
		hInstance,
		NULL
	);
	g_hPathTextboxInv = CreateWindowW(
		L"EDIT",
		L"",
		WS_VISIBLE | WS_CHILD | ES_READONLY | WS_BORDER,
		10, 520, 200, 20,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hComputerInfoLabel = CreateWindowW(
		L"STATIC",
		L"Computer Information will be displayed here:",
		WS_VISIBLE | WS_CHILD,
		330, 10, 400, 50,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hHeaderInfoLabel = CreateWindowW(
		L"STATIC",
		L"Bitmap Information:",
		WS_VISIBLE | WS_CHILD,
		750, 10, 400, 20,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hFeatureBox = CreateWindowW(
		L"EDIT",
		L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY,
		330, 50, 400, 400,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hHeaderInfoBox = CreateWindowW(
		L"EDIT",
		L"",
		WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY,
		750, 50, 400, 400,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	g_hTestComboBox = CreateWindowW(
		L"COMBOBOX",
		NULL,
		CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
		330, 480, 120, 400,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	for (const auto& testName : g_testNames) {
		SendMessageW(g_hTestComboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(testName.c_str()));
	}

	SendMessageW(g_hTestComboBox, CB_SETCURSEL, 0, 0);

	g_hRunTimesListBox = CreateWindowW(
		L"LISTBOX",
		NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY,
		480, 480, 200, 100,
		hwnd,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(hwnd, nCmdShow);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		if (reinterpret_cast<HWND>(lParam) == g_hButton) {
			OpenImageDialog(hwnd);
			DisplayHeaderInfo(hwnd);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void OpenImageDialog(HWND hwnd) {
	OPENFILENAMEW ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Image Files\0*.bmp;*.png;*.jpg;*.jpeg;*.gif\0All Files\0*.*\0";
	ofn.lpstrFile = g_szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open Image";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (GetOpenFileNameW(&ofn)) {
		HBITMAP hBitmap = (HBITMAP)LoadImageW(NULL, g_szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

		if (hBitmap == NULL) {
			MessageBoxW(hwnd, L"Failed to load image.", L"Error", MB_ICONERROR);
			return;
		}

		HDC hdcImage = CreateCompatibleDC(NULL);
		SelectObject(hdcImage, hBitmap);

		BITMAP bm;
		GetObject(hBitmap, sizeof(bm), &bm);

		SetStretchBltMode(GetDC(g_hImageBox), COLORONCOLOR);

		StretchBlt(GetDC(g_hImageBox), 0, 0, 280, 280, hdcImage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

		SetWindowTextW(g_hPathTextbox, g_szFileName);

		SetWindowTextW(g_hPathTextboxInv, g_szFileName);

		DeleteDC(hdcImage);
		DeleteObject(hBitmap);
	}
}

void DisplayHeaderInfo(HWND hwnd) {

	std::wstring headerInfoText = L"Sample Bitmap Header Info\nWidth: 800\nHeight: 600\n";
	SetWindowTextW(g_hHeaderInfoBox, headerInfoText.c_str());
}

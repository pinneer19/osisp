#include <Windows.h>


int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {

	const wchar_t CLASS_NAME[] = L"Window Class Name";
	
	WNDCLASS wnd = {};
	wnd.lpszClassName = CLASS_NAME;
	wnd.hInstance = hInst;
	wnd.lpfnWndProc = WindowsProcedure;
	
	RegisterClass(&wnd);
	HWND hWnd = CreateWindowEx(0, CLASS_NAME, L"Learn to Program", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL);
	if (hWnd == NULL) {
		return 0;
	}
}
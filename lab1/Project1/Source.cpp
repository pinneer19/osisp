#include <Windows.h>
#include "Minesweeper.cpp"
#include <commctrl.h>

void AddMenu(HWND);
void updateMode(Mode, HWND, int, int);
void AddMinesweeperField(HWND hWnd);
void CALLBACK UpdateField(short i, short j, int size, HWND hWnd);
void CALLBACK OpenCell(std::pair<short, short> ind, HWND hWnd);
std::pair<short, short> getButtonIndices(short id);

#define MENU_ACTION_1 1
#define MENU_ACTION_2 2
#define MENU_ACTION_3 3
#define MENU_ACTION_4 4
#define IDC_FIRST_BUTTON 5

Minesweeper mine = Minesweeper();

LRESULT CALLBACK ButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	switch (uMsg) {
	case WM_RBUTTONDOWN:
	{
		int buttonID = GetDlgCtrlID(hWnd);
		std::pair<short, short> indices = getButtonIndices(buttonID);
		wchar_t buttonStatus = mine.setAsMine(indices.first, indices.second);
		if(buttonStatus == '.') SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(L""));
		else if(buttonStatus == '*') {		
			SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)(L"#"));
			if (mine.checkWinner()) {
				MessageBox(hWnd, L"You won!", L"Game status", MB_OK);
				PostQuitMessage(0);
			}
		}
		break;
	}
	default:
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		switch (wParam) {
		case MENU_ACTION_1:
			if (mine.getMode() != BEGINNER) updateMode(BEGINNER, hWnd, 27, 28);
			break;
		case MENU_ACTION_2:
		{
			if (mine.getMode() != ADVANCED) updateMode(ADVANCED, hWnd, 50, 75);
			break;
		}
		case MENU_ACTION_3:
		{
			if (mine.getMode() != ADVANCED) updateMode(EXPERT, hWnd, 120, 120);
			break;
		}
		case MENU_ACTION_4:
		{
			int result = MessageBox(hWnd, L"Are you sure?", L"Exit", MB_YESNO);
			if (result == IDYES) {
				PostQuitMessage(0);
			}
			break;
		}
		default: {
			int id = LOWORD(wParam);
			HWND button = GetDlgItem(hWnd, id);
			OpenCell(getButtonIndices(id), hWnd);
			break;
		}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		AddMenu(hWnd);
		AddMinesweeperField(hWnd);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


WNDCLASS NewWindowClass(HINSTANCE hInstance) {

	WNDCLASS wc{ sizeof(WNDCLASS) };
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpszClassName = L"App";
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszMenuName = nullptr;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	return wc;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	MSG msg{};
	HWND hwnd{};

	WNDCLASS wc = NewWindowClass(hInstance);

	if (!RegisterClass(&wc)) {
		return EXIT_FAILURE;
	}
	hwnd = CreateWindow(wc.lpszClassName, L"Header", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, mine.getFieldSize() * 45 - 27, mine.getFieldSize() * 50 - 28, nullptr, nullptr, wc.hInstance, nullptr);
	if (hwnd == INVALID_HANDLE_VALUE) {
		return EXIT_FAILURE;
	}
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return EXIT_SUCCESS;
}

void AddMenu(HWND hWnd) {
	HMENU rootMenu = CreateMenu();
	HMENU subMenu = CreateMenu();
		
	AppendMenu(subMenu, MF_STRING, MENU_ACTION_1, L"Beginner");
	AppendMenu(subMenu, MF_STRING, MENU_ACTION_2, L"Advanced");
	AppendMenu(subMenu, MF_STRING, MENU_ACTION_3, L"Expert");
	AppendMenu(subMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(subMenu, MF_STRING, MENU_ACTION_4, L"Exit");

	AppendMenu(rootMenu, MF_POPUP, (UINT_PTR)subMenu, L"Mode");

	SetMenu(hWnd, rootMenu);
}


void AddMinesweeperField(HWND hWnd) {

	int size = mine.getFieldSize();
	for (int i = 0; i < size; ++i) {	
		for (int j = 0; j < size; ++j) {

			int buttonId = IDC_FIRST_BUTTON + i * size + j;
			HWND button = CreateWindow(L"BUTTON", L"", WS_VISIBLE | WS_CHILD, 40 * j, 40 * i, 40, 40, hWnd, reinterpret_cast<HMENU>(buttonId), NULL, NULL);
			SetWindowSubclass(button, ButtonProc, 0, 0);
		}
	}
}



void CALLBACK OpenCell(std::pair<short, short> ind, HWND hWnd) {

	// end game
	if (!mine.openCell(ind.first, ind.second)) {
		MessageBox(hWnd, L"You lose", L"Game status", MB_OK);
		PostQuitMessage(0);
	}
	else
	{
		// open cell	
		UpdateField(ind.first, ind.second, mine.getFieldSize(), hWnd);
	}
}

std::pair<short, short> getButtonIndices(short id) {
	
	short row_index = (id - 5) / mine.getFieldSize();
	short column_index = (id - 5) % mine.getFieldSize();

	return std::pair<short, short>(row_index, column_index);
}


void CALLBACK UpdateField(short i, short j, int size, HWND hWnd) {
	wchar_t status = mine.getStatus(i, j);
	
	if (status == '/') return;
	
	HWND button = GetDlgItem(hWnd, IDC_FIRST_BUTTON + i * size + j);
	
	wchar_t wcharDigit[2];
	wcharDigit[0] = mine.checkCell(i, j);
	wcharDigit[1] = L'\0';
	if (mine.checkWinner()) {
		MessageBox(hWnd, L"You won!", L"Game status", MB_OK);
		PostQuitMessage(0);
	}
	SendMessage(button, WM_SETTEXT, 0, (LPARAM)(wcharDigit));
	if (wcharDigit[0] == '/') {
		for (short x = max(i - 1, 0); x <= min(i + 1, size - 1); ++x) {
			for (short y = max(j - 1, 0); y <= min(j + 1, size - 1); ++y) {

				if (x != i || y != j) {
					UpdateField(x, y, size, hWnd);
				}
			}
		}
	}
	
	
}

void updateMode(Mode mode, HWND hWnd, int dx, int dy) {
	HWND button;
	for (int i = 0; i < mine.getFieldSize(); ++i) {
		for (int j = 0; j < mine.getFieldSize(); ++j) {
			int buttonId = IDC_FIRST_BUTTON + i * mine.getFieldSize() + j;
			button = GetDlgItem(hWnd, buttonId);
			DestroyWindow(button);
		}
	}

	mine = Minesweeper(mode);

	RECT clientRect = { 0, 0, 600, 600 };

	int newWidth = mine.getFieldSize() * 45 - dx;
	int newHeight = mine.getFieldSize() * 50 - dy;
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
	SetWindowPos(hWnd, nullptr, 0, 0, newWidth, newHeight, SWP_NOMOVE | SWP_NOZORDER);
	UpdateWindow(hWnd);
	AddMinesweeperField(hWnd);
}
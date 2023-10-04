#include "windows.h"
#include "resource.h"
#include "cmath"

WNDCLASS MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
enum Path {
    STRAIGHT, SINUS, ELLIPSIS
};

// global vars
RECT clientRect;
RECT object = {100, 250, 25, 25};
HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // White brush
HBRUSH grayBrush = CreateSolidBrush(RGB(127, 127, 127)); // Gray brush
HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255)); // Blue brush
bool forward = true;
bool isAnimating = false;
float speed = 1;
Path path = STRAIGHT;

#define BTN_1 1
#define BTN_2 2
#define BTN_3 3
#define BTN_4 4
#define BTN_5 5
#define BTN_6 6
#define BTN_7 7
HWND btn1 = nullptr, btn2 = nullptr, btn3 = nullptr, btn4 = nullptr, btn5 = nullptr, btn6 = nullptr, btn7 = nullptr;

void ClearPosition(HWND hWnd) {
    isAnimating = false;
    SetWindowText(GetDlgItem(hWnd, BTN_1), L"\u25B6");
    object = { 100, 250, 25, 25 };
    speed = 1;
}
int counter = 0;
void moveArray(Path path = STRAIGHT) {

    if (path == STRAIGHT) {
        if (forward) object.left += 3;
        else object.left -= 3;
        if (object.left >= clientRect.right - clientRect.left - 50) forward = false;
        if (object.left <= 0) forward = true;
        object.right = object.left + 25;
        object.bottom = object.top + 25;
    }
    else if (path == SINUS) {
        double amplitude = 50.0; // Amplitude of the sinusoidal wave
        double frequency = 0.05; // Frequency of the sinusoidal wave

        // Calculate the new horizontal position based on the sine wave
        if (forward) {
            object.left += 3;
        }
        else {
            object.left -= 3;
        }

        // Calculate the corresponding vertical position using the sine wave formula
        object.top = static_cast<LONG>(amplitude * sin(frequency * object.left)) + 195;

        object.right = object.left + 25;
        object.bottom = object.top + 25;

        if (object.left >= clientRect.right - clientRect.left - 50) {
            forward = false;
        }
        else if (object.left <= 0) {
            forward = true;
        }
    }
    else if (path == ELLIPSIS) {
        
        double a = 400.0; // Horizontal radius
        double b = 100.0;  // Vertical radius
        double angle = counter * 0.05;

        double x = a * cos(angle);
        double y = b * sin(angle); // Parametric equation for an ellipse
        object.left = static_cast<LONG>(x) + (clientRect.right - clientRect.left) / 2;
        object.top = static_cast<LONG>(y) + (clientRect.bottom - clientRect.top) / 2;

        // Update the ellipse's right and bottom coordinates
        object.right = object.left + 25;
        object.bottom = object.top + 25;

        counter++;

        if (counter >= 628) {
            counter = 0;
        }
    }
}

void ShowRectangle(HDC dc) {
    SelectObject(dc, grayBrush);
    HDC memDC = CreateCompatibleDC(dc);
    HBITMAP memBM = CreateCompatibleBitmap(dc, clientRect.right - clientRect.left - 20, clientRect.bottom - clientRect.top - 100);
    SelectObject(memDC, memBM);
    
    HGDIOBJ oldBrush = SelectObject(memDC, whiteBrush);
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255,0,0));
    HGDIOBJ oldPen = SelectObject(memDC, pen);
    Rectangle(memDC, clientRect.left, clientRect.top, clientRect.right - 20, clientRect.bottom - 100);
    SelectObject(memDC, oldBrush);
    SelectObject(memDC, oldPen);
    DeleteObject(pen);

    oldBrush = SelectObject(memDC, blueBrush);

    Ellipse(memDC, object.left, object.top, object.right, object.bottom);
    SelectObject(memDC, oldBrush);

    BitBlt(dc, 10, 10, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, memDC, 0,0, SRCCOPY);
    
    DeleteDC(memDC);
    DeleteObject(memBM);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow)
{

    WNDCLASS wc = MyRegisterClass(hInstance);
    if (!RegisterClass(&wc)) {
        return EXIT_FAILURE;
    }

    HWND hWnd = CreateWindow(wc.lpszClassName, L"Animation", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 640, nullptr, nullptr, wc.hInstance, nullptr);

    if (hWnd == INVALID_HANDLE_VALUE)
    {
        return EXIT_FAILURE;
    }


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    HDC dc = GetDC(hWnd);
    MSG msg;
    // ÷икл основного сообщени€:
    
    while (1)
    {
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
        }
        else {
            if (isAnimating) {
                moveArray(path);
                ShowRectangle(dc);
                if (path == STRAIGHT) {
                    if (speed == 1) Sleep(10);
                    else if (speed == 0.5) Sleep(30);
                }
                else if (path == ELLIPSIS) {
                    if (speed == 1) Sleep(15);
                    else if (speed == 0.5) Sleep(30);
                    else Sleep(1);
                }
                else if (path == SINUS) {
                    if (speed == 1) Sleep(10);
                    else if (speed == 0.5) Sleep(25);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

//
//  ‘”Ќ ÷»я: MyRegisterClass()
//
//  ÷≈Ћ№: –егистрирует класс окна.
//
WNDCLASS MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASS wcex{ sizeof(WNDCLASS) };

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"CLASSNAME";

    return wcex;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case BTN_1:
        {
            isAnimating = !isAnimating;

            if (isAnimating) {
                SetWindowText(GetDlgItem(hWnd, BTN_1), L"| |");
            }
            else {
                SetWindowText(GetDlgItem(hWnd, BTN_1), L"\u25B6");
            }

            break;
        }
        case BTN_2:
        {
            speed = 0.5;
            break;
        }
        case BTN_3:
        {
            speed = 1;
            break;
        }
        case BTN_4:
        {
            speed = 2;
            break;
        }
        case BTN_5:
        {
            path = STRAIGHT;
            ClearPosition(hWnd);
            break;
        }
        case BTN_6:
        {
            path = SINUS;
            ClearPosition(hWnd);
            break;
        }
        case BTN_7:
        {
            path = ELLIPSIS;
            ClearPosition(hWnd);
            break;
        }
        }
        break;
    
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        GetClientRect(hWnd, &clientRect); // get window's client area coordinates; clientRect - data receiver
        // WS_VISIBLE - show button
        if (btn1 != nullptr) {
            DestroyWindow(btn1);
            btn1 = nullptr;
        }
        if (btn2 != nullptr) {
            DestroyWindow(btn2);
            btn2 = nullptr;
        }
        if (btn3 != nullptr) {
            DestroyWindow(btn3);
            btn3 = nullptr;
        }
        if (btn4 != nullptr) {
            DestroyWindow(btn4);
            btn4 = nullptr;
        }
        if (btn5 != nullptr) {
            DestroyWindow(btn5);
            btn5 = nullptr;
        }
        if (btn6 != nullptr) {
            DestroyWindow(btn6);
            btn6 = nullptr;
        }
        if (btn7 != nullptr) {
            DestroyWindow(btn7);
            btn7 = nullptr;
        }

        btn1 = CreateWindow(L"BUTTON", L"\u25B6", WS_VISIBLE | WS_CHILD, clientRect.left + 20, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_1), NULL, NULL);   
        btn2 = CreateWindow(L"BUTTON", L"0.5x", WS_VISIBLE | WS_CHILD, clientRect.left + 100, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_2), NULL, NULL);
        btn3 = CreateWindow(L"BUTTON", L"1x", WS_VISIBLE | WS_CHILD, clientRect.left + 180, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_3), NULL, NULL);
        btn4 = CreateWindow(L"BUTTON", L"2x", WS_VISIBLE | WS_CHILD, clientRect.left + 260, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_4), NULL, NULL);
        btn5 = CreateWindow(L"BUTTON", L"Straight", WS_VISIBLE | WS_CHILD, clientRect.left + 340, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_5), NULL, NULL);
        btn6 = CreateWindow(L"BUTTON", L"Sinus", WS_VISIBLE | WS_CHILD, clientRect.left + 420, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_6), NULL, NULL);
        btn7 = CreateWindow(L"BUTTON", L"Ellipsis", WS_VISIBLE | WS_CHILD, clientRect.left + 500, clientRect.bottom - 80, 70, 70,
            hWnd, reinterpret_cast<HMENU>(BTN_7), NULL, NULL);
        break;
    case WM_PAINT:
    {   
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps); // handle to context device
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        Rectangle(hdc, clientRect.left + 10, clientRect.top + 10, clientRect.right - 20, clientRect.bottom - 90);
        EndPaint(hWnd, &ps);
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
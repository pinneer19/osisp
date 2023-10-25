#include <string>
#include "framework.h"
#include "lab34.h"
#include "tlhelp32.h"
#include <commctrl.h>
#include "psapi.h"
#define MAX_LOADSTRING 100

#pragma comment(lib, "comctl32.lib")

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
#define ID_LISTVIEW 100
HWND hWndListView;
bool isAscending = true;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND addButton(HWND hWnd, int x, int y, int width, int height, int id, LPCWSTR buttonText);

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM sortParam)
{   
    int pid1 = static_cast<int>(lParam1);
    int pid2 = static_cast<int>(lParam2);
    if (isAscending)
        return pid1 - pid2; // Compare PIDs in ascending order
    else
        return pid2 - pid1; // Compare PIDs in descending order
}



HWND CreateListView(HWND hwndParent)
{
    RECT rcClient; // The parent window's client area.

    GetClientRect(hwndParent, &rcClient);
    InitCommonControls();

    // Create the list-view window in report view with label editing enabled.
    hWndListView = CreateWindow(WC_LISTVIEW,
        L"", // window name
        // child window, first column of list view is left-aligned 
        WS_CHILD | LVS_REPORT | WS_VISIBLE,
        0, 0,
        rcClient.right - rcClient.left,
        rcClient.bottom - rcClient.top,
        hwndParent,
        (HMENU)ID_LISTVIEW,
        hInst,
        NULL);

    ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP);


    return (hWndListView);
}


// InitListViewColumns: Adds columns to a list-view control.
// hWndListView:        Handle to the list-view control. 
// Returns TRUE if successful, and FALSE otherwise. 
BOOL InitListViewColumns(HWND hWndListView)
{
    WCHAR szText[256];     // Temporary buffer.
    LVCOLUMN lvc;
    int iCol;

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text,
    // and subitem members of the structure are valid.
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    // set which members contain valid information
    // Add the columns.
    for (iCol = 0; iCol < 4; iCol++)
    {
        lvc.iSubItem = iCol;
        lvc.pszText = szText;
        lvc.cx = 100;               // Width of column in pixels.

        if (iCol < 2)
            lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.
        else
            lvc.fmt = LVCFMT_RIGHT; // Right-aligned column.

        // Load the names of the column headings from the string resources.
        LoadString(hInst,
            IDS_STRING104 + iCol,
            szText,
            sizeof(szText) / sizeof(szText[0]));

        // Insert the columns into the list view.
        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
            return FALSE;
    }

    return TRUE;
}


PROCESS_MEMORY_COUNTERS GetMemoryInfo(DWORD processID)
{
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc; // structure the contains memory info for a process
    //  flags:                certain process info,       read info in process                           
    // which are required for getprocessmemoryinfo call
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    
    if (hProcess != NULL) {
        // Retrieving information about the memory usage of the specified process.
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
        {
            CloseHandle(hProcess);
            return pmc;
        }
        else {
            CloseHandle(hProcess);
        }
    }
    return pmc;

}

void SuspendProcess(DWORD processID) {
    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &threadEntry);

    do
    {
        if (threadEntry.th32OwnerProcessID == processID)
        {
            // get handle for existing thread object
            // false - the processes created by this process do not inherit this handle.
            // THREAD_SUSPEND_RESUME - thread access right: Required to suspend or resume a thread 
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadEntry.th32ThreadID);

            SuspendThread(hThread); // return value is the thread's previous suspend count
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &threadEntry));

    CloseHandle(hThreadSnapshot);
    MessageBox(NULL, L"Process was suspended!", L"OK", MB_OK);

}

// get running processes using Toolhelp32 library 
void GetRunningProcesses(HWND hWndListView) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32; // structure describes entry from processes list

    // Take a snapshot of all processes in the system.
    // th32cs_snapprocess flag includes all system processes in the snapshot, 0 is id of process (used with other flags)
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Delete all items in the list view.
    ListView_DeleteAllItems(hWndListView);

    // Now walk the snapshot of processes and populate the list view.
    if (Process32First(hProcessSnap, &pe32)) { // extract first process info fron snap to pe32 structure

        PROCESS_MEMORY_COUNTERS pmc;
        pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS);
        int i = 0;
        do {
            // Add information about the process to the list view.
            LVITEM lvItem;
            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.iItem = i; // Index of the new item
            lvItem.iSubItem = 0; // Index of the sub-item (first column).
            lvItem.pszText = pe32.szExeFile; // Process name.
            lvItem.lParam = pe32.th32ProcessID;
            int nIndex = ListView_InsertItem(hWndListView, &lvItem);

            // Display the PID in the second column.
            WCHAR pidBuffer[16];
            swprintf_s(pidBuffer, L"%lu", pe32.th32ProcessID);
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = nIndex;
            lvItem.iSubItem = 1; // Index of the sub-item (second column).
            lvItem.pszText = pidBuffer;            
            ListView_SetItem(hWndListView, &lvItem);

            // Display the number of threads in the third column.
            WCHAR threadsBuffer[16];
            swprintf_s(threadsBuffer, L"%lu", pe32.cntThreads);
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = nIndex;
            lvItem.iSubItem = 2; // Index of the sub-item (third column).
            lvItem.pszText = threadsBuffer;
            ListView_SetItem(hWndListView, &lvItem);

            // Display process memory in fourth column
            pmc = GetMemoryInfo(pe32.th32ProcessID);
            lvItem.mask = LVIF_TEXT;
            lvItem.iItem = nIndex;
            lvItem.iSubItem = 3; // Index of the sub-item (fourth column).
            
            double memoryUsageMB = static_cast<double>(pmc.WorkingSetSize) / 1048576.0; // 1 megabyte = 1048576 bytes
            
            WCHAR buffer[32];
            swprintf_s(buffer, L"%.2f", memoryUsageMB);

            lvItem.pszText = buffer;
            
            ListView_SetItem(hWndListView, &lvItem);

        } while (Process32Next(hProcessSnap, &pe32)); // retrieve info about next process in snapshot
    }

    // Clean up the snapshot object.
    CloseHandle(hProcessSnap);
}

void TerminateProcess(DWORD processID) {

    // Open the process with PROCESS_TERMINATE access right.
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID);

    if (hProcess != NULL) {
        // Terminate the process.
        if (TerminateProcess(hProcess, 0)) {
            MessageBox(NULL, L"Process was terminated!", L"OK", MB_OK);
            GetRunningProcesses(hWndListView);
        }
        else {
            // TerminateProcess failed, handle the error.
            DWORD error = GetLastError();
            MessageBox(NULL, L"Process wasn't terminated!", L"OK", MB_ICONERROR | MB_OK);
        }

        // Close the process handle.
        CloseHandle(hProcess);
    }
    else {
        // OpenProcess failed, handle the error.
        DWORD error = GetLastError();
        // Handle the error, e.g., log it or display a message.
    }
}

void ResumeProcess(DWORD processID) {

    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    Thread32First(hThreadSnapshot, &threadEntry);

    do
    {
        if (threadEntry.th32OwnerProcessID == processID)
        {
            // get handle for existing thread object
            // false - the processes created by this process do not inherit this handle.
            // THREAD_SUSPEND_RESUME - thread access right: Required to suspend or resume a thread 
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadEntry.th32ThreadID);

            ResumeThread(hThread); // return value is the thread's previous suspend count
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnapshot, &threadEntry));

    CloseHandle(hThreadSnapshot);
    MessageBox(NULL, L"Process was resumed!", L"OK", MB_OK);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    
    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB34, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;
    
    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);   
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB34));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB34);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 600, 800, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int GetSelectedListViewItem() {
    // Get the index of the selected item in the ListView.
    int selectedIndex = ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED);

    // Return the index of the selected item.
    return selectedIndex;
}

int GetProcessId(int selectedIndex) {
    LVITEM lvItem = { 0 };
    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = selectedIndex;

    if (ListView_GetItem(hWndListView, &lvItem)) {
        return static_cast<DWORD>(lvItem.lParam);
    }

    // Handle errors
    return 0;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        CreateListView(hWnd);
        InitListViewColumns(hWndListView);
        GetRunningProcesses(hWndListView);
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Разобрать выбор в меню:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_SUSPEND_PROCESS:
        {
            int index = GetSelectedListViewItem();

            if (index == -1) {
                MessageBox(hWnd, L"Please select process!", L"OK", MB_OK);
                break;
            }
            SuspendProcess(GetProcessId(index));

            break;
        }
        case IDM_RESUME_PROCESS:
        {   int index = GetSelectedListViewItem();

            if (index == -1) {
                MessageBox(hWnd, L"Please select process!", L"OK", MB_OK);
                break;
            }
            ResumeProcess(GetProcessId(index));
            break;
        }
        case IDM_TERMINATE_PROCESS:
        {
            int index = GetSelectedListViewItem();

            if (index == -1) {
                MessageBox(hWnd, L"Please select process!", L"OK", MB_OK);
                break;
            }
            TerminateProcess(GetProcessId(index));

            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_NOTIFY:
    {
        LPNMHDR tc;
        LPNMLISTVIEW pnmlv;
        DWORD subitm;

        tc = (LPNMHDR)lParam;

        if ((tc->hwndFrom == hWndListView) && (tc->code == LVN_COLUMNCLICK)) {
            pnmlv = (LPNMLISTVIEW)lParam;
            subitm = pnmlv->iSubItem;
            ListView_SortItems(hWndListView, CompareFunc, subitm);
            isAscending = !isAscending;
        }
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

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{    
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

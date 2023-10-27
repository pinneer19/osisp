#include "framework.h"
#include "lab5.h"
#include <commctrl.h>
#include <string>
#define MAX_LOADSTRING 100
#pragma comment(lib, "comctl32.lib")

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
#define ID_LISTVIEW 100
#define ID_BUTTON_CREATE_KEY    555
#define ID_BUTTON_DELETE_KEY    556
#define ID_BUTTON_EDIT_KEY      557
#define ID_BUTTON_ADD_VALUE     558
#define ID_BUTTON_EDIT_VALUE    559
#define ID_BUTTON_DELETE_VALUE  560
#define ID_EDIT_NAME            561
#define ID_EDIT_DATA1           562
#define ID_EDIT_DATA2           563


HWND hTreeView;
HWND editText;
HWND btn_create_key = NULL, btn_delete_key = NULL, btn_edit_key = NULL, btn_add_value = NULL,
    btn_edit_value = NULL, btn_delete_value = NULL, editName = NULL, editData1 = NULL, editData2 = NULL;
// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


void LoadRegistryTree(HTREEITEM parent, HKEY root)
{
    DWORD index = 0;
    TCHAR name[MAX_PATH];

    while (RegEnumKey(root, index++, name, sizeof(name) / sizeof(name[0])) == ERROR_SUCCESS) {
        TV_INSERTSTRUCT tvinsert = { 0 };
        tvinsert.hParent = parent;
        tvinsert.hInsertAfter = TVI_LAST;
        tvinsert.item.mask = TVIF_TEXT;
        tvinsert.item.pszText = name;
        HTREEITEM node = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
        HKEY next_key;
        RegOpenKey(root, name, &next_key);
        LoadRegistryTree(node, next_key);
        RegCloseKey(next_key);
    }

    
    index = 0;
    DWORD valueNameLength = MAX_PATH;
    DWORD valueDataLength = sizeof(BYTE) * MAX_PATH;
    DWORD valueType;
    BYTE  lpData[MAX_PATH];
    while (RegEnumValue(root, index++, name, &valueNameLength, NULL, &valueType, lpData, &valueDataLength) == ERROR_SUCCESS) {
        TV_INSERTSTRUCT tvinsert = { 0 };
        tvinsert.hParent = parent;
        tvinsert.hInsertAfter = TVI_LAST;
        tvinsert.item.mask = TVIF_TEXT;
        std::wstring dataString = reinterpret_cast<wchar_t*>(lpData);
        std::wstring itemText = name;
        itemText += L" Type: REG_SZ Value: " + dataString;
        
        tvinsert.item.pszText = const_cast<LPWSTR>(itemText.c_str());
        
        HTREEITEM node = (HTREEITEM)SendMessage(hTreeView, TVM_INSERTITEM, 0, (LPARAM)&tvinsert);
        valueNameLength = MAX_PATH;
        valueDataLength = sizeof(BYTE) * MAX_PATH;
    }
}
void LoadRegistry()
{
    TreeView_DeleteAllItems(hTreeView);
    HKEY root;
    RegOpenKey(HKEY_CURRENT_USER, L"Software\\Lab5", &root);
    LoadRegistryTree(NULL, root);
    RegCloseKey(root);
}

void AddRegistryKey(LPCWSTR keyName, LPCWSTR keyPath = NULL) {
    HKEY hKey = NULL;

    std::wstring fullKey = L"Software\\Lab5\\";
    // Combine the subKey with the keyName
    if (keyPath != NULL) fullKey += keyPath;


    // HKEY_CURRENT_USER - selected registry key
    // next parameter is optional name of the registry subkey to be opened
    // 0 is option applied when opening the key (it is used like default)
    // KEY_READ specifies access rights
    // last is result handle receiver for opened key as ERROR_SUCESS result
    
    if (RegOpenKeyEx(HKEY_CURRENT_USER, fullKey.c_str(), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        HKEY result = NULL;

        if (RegOpenKeyEx(hKey, keyName, 0, KEY_READ, &result) == ERROR_SUCCESS) MessageBox(NULL, L"Key already exists!", L"Info", MB_OK);
        // key handle, name for key, hkey reveiver
        else if (RegCreateKeyEx(hKey, keyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &result, NULL) != ERROR_SUCCESS) {
            MessageBox(NULL, L"Key was not created", L"Info", MB_OK);
        }
    }
    RegCloseKey(hKey);

    LoadRegistry();
}

void AddRegistryValue(LPCWSTR keyName, LPCWSTR valueName, LPCWSTR valueData, LPCWSTR keyPath = NULL) {
    HKEY hKey = NULL;
    
    std::wstring fullKey = L"Software\\Lab5\\";
    if (keyPath != NULL) fullKey += keyPath;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, fullKey.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegSetKeyValue(hKey, keyName, valueName, REG_SZ, reinterpret_cast<LPCVOID>(valueData), (wcslen(valueData) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS) {
            MessageBox(NULL, L"Value added successfully!", L"Info", MB_OK);
        }
        else {
            MessageBox(NULL, L"Failed to add value", L"Error", MB_OK);
        }
        RegCloseKey(hKey);
        LoadRegistry();
    }
    else {
        MessageBox(NULL, L"Failed to open key", L"Error", MB_OK);
    }
}

void EditRegistryValue(LPCWSTR keyName, LPCWSTR valueName, LPCWSTR newValueData, LPCWSTR keyPath = NULL) {
    HKEY hKey = NULL;

    std::wstring fullKey = L"Software\\Lab5\\";
    if (keyPath != NULL) fullKey += keyPath;
    fullKey = fullKey.substr(0, fullKey.find_last_of(L"\\"));
    if (RegOpenKeyEx(HKEY_CURRENT_USER, fullKey.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegSetKeyValue(hKey, keyName, valueName, REG_SZ, (const BYTE*)newValueData, (wcslen(newValueData) + 1) * sizeof(wchar_t)) == ERROR_SUCCESS) {
            MessageBox(NULL, L"Value edited successfully!", L"Info", MB_OK);
        }
        else {
            MessageBox(NULL, L"Failed to edit value", L"Error", MB_OK);
        }
        RegCloseKey(hKey);
        LoadRegistry();
    }
    else {
        MessageBox(NULL, L"Failed to open key", L"Error", MB_OK);
    }
}

void DeleteRegistryValue(LPCWSTR valueName, LPCWSTR keyPath = NULL) {
    HKEY hKey = NULL;
    
    std::wstring fullKey = L"Software\\Lab5\\";
    if(keyPath != NULL) fullKey += keyPath;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, fullKey.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (RegDeleteValue(hKey, valueName) == ERROR_SUCCESS) {
            MessageBox(NULL, L"Value deleted successfully!", L"Info", MB_OK);
        }
        else {
            MessageBox(NULL, L"Failed to delete value", L"Error", MB_OK);
        }
        RegCloseKey(hKey);
        LoadRegistry();
    }
    else {
        MessageBox(NULL, L"Failed to open the 'Software' key", L"Error", MB_OK);
    }


}

void DeleteRegistryKey(LPCWSTR keyName, LPCWSTR keyPath = NULL) {

    std::wstring fullKey = L"Software\\Lab5\\";
    if (keyPath != NULL) fullKey += keyPath;

    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, fullKey.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        HKEY result = NULL;

        // key handle, name for key, hkey reveiver
        if (RegDeleteKey(hKey, keyName) != ERROR_SUCCESS) {
            
            MessageBox(NULL, L"Key was not deleted", L"Info", MB_OK);
        }
    }
    RegCloseKey(hKey);
    LoadRegistry();
}

void EditRegistryKey(LPCWSTR keyName, LPCWSTR newKeyName, LPCWSTR keyPath = NULL) {

    std::wstring fullKey = L"Software\\Lab5\\";
    if (keyPath != NULL) fullKey += keyPath;
    
    HKEY hKey = NULL;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, fullKey.c_str(), 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        HKEY result = NULL;

        // key handle, name for key, hkey reveiver
        if (RegRenameKey(hKey, keyName, newKeyName) != ERROR_SUCCESS) {
            MessageBox(NULL, L"Key was not updated", L"Info", MB_OK);
        }
    }
    RegCloseKey(hKey);
    LoadRegistry();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB5, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB5));

    MSG msg;

    // Цикл основного сообщения:
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB5));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB5);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

bool GetSelectedItemFullPath(std::wstring* pathBuffer, std::wstring* keyNameBuffer, std::wstring* valueKeyNameBuffer = nullptr) {
    HTREEITEM hSelectedItem = TreeView_GetSelection(hTreeView); // Get the selected item.
    pathBuffer->clear();
    if (hSelectedItem) {
        TVITEM tvi = { 0 };
        tvi.mask = TVIF_TEXT;
        tvi.hItem = hSelectedItem;
        tvi.cchTextMax = sizeof(TCHAR) * MAX_PATH;
        tvi.pszText = new TCHAR[MAX_PATH];

        TreeView_GetItem(hTreeView, &tvi); // Get the item's text.
        keyNameBuffer->assign(tvi.pszText);
        
        do {
            
            hSelectedItem = TreeView_GetParent(hTreeView, hSelectedItem); // Get the parent item.
            
            // returns null if it reached root of tree
            if (!hSelectedItem) {
                //if(pathBuffer->empty()) return false;
                return true;
            }
            tvi.hItem = hSelectedItem;
            TreeView_GetItem(hTreeView, &tvi); // Get the item's text.
            if (valueKeyNameBuffer != nullptr && valueKeyNameBuffer->empty()) valueKeyNameBuffer->assign(tvi.pszText);
            if (!pathBuffer->empty()) {
                std::wstring tmp = *pathBuffer;
                pathBuffer->assign(tvi.pszText);
                pathBuffer->append(L"\\");
                pathBuffer->append(tmp);
            }
            else {
                pathBuffer->assign(tvi.pszText);
            }

            
        } while (hSelectedItem);

        return true;    
    }

    return false;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 600, 600, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

bool checkNameField(WCHAR* buffer) {

    GetWindowText(editName, buffer, 256);

    if (_tcslen(buffer) == 0)
    {
        MessageBox(NULL, L"Key name cannot be empty!", L"Info", MB_OK);
        return false;
    }
    return true;
}

bool checkDataField_1(WCHAR* buffer) {
    GetWindowText(editData1, buffer, 256);

    if (_tcslen(buffer) == 0)
    {
        MessageBox(NULL, L"Data field cannot be empty!", L"Info", MB_OK);
        return false;
    }
    return true;
}

bool checkDataField_2(WCHAR* buffer) {
    GetWindowText(editData2, buffer, 256);

    if (_tcslen(buffer) == 0)
    {
        MessageBox(NULL, L"Data field cannot be empty!", L"Info", MB_OK);
        return false;
    }
    return true;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {   
        
        RECT rcClient; // The parent window's client area.

        GetClientRect(hWnd, &rcClient);
        InitCommonControls();

        btn_create_key = CreateWindow(WC_BUTTON, L"Add key", WS_CHILD | WS_VISIBLE, rcClient.left + 320 , rcClient.top + 10, 120, 60, hWnd, (HMENU)ID_BUTTON_CREATE_KEY, hInst, NULL);
        btn_delete_key = CreateWindow(WC_BUTTON, L"Delete key", WS_CHILD | WS_VISIBLE, rcClient.left + 320, rcClient.top + 80, 120, 60, hWnd, (HMENU)ID_BUTTON_DELETE_KEY, hInst, NULL);
        btn_edit_key = CreateWindow(WC_BUTTON, L"Edit key", WS_CHILD | WS_VISIBLE, rcClient.left + 320, rcClient.top + 150, 120, 60, hWnd, (HMENU)ID_BUTTON_EDIT_KEY, hInst, NULL);
        btn_add_value = CreateWindow(WC_BUTTON, L"Add value", WS_CHILD | WS_VISIBLE, rcClient.left + 320, rcClient.top + 220, 120, 60, hWnd, (HMENU)ID_BUTTON_ADD_VALUE, hInst, NULL);
        btn_delete_value = CreateWindow(WC_BUTTON, L"Delete value", WS_CHILD | WS_VISIBLE, rcClient.left + 320, rcClient.top + 290, 120, 60, hWnd, (HMENU)ID_BUTTON_DELETE_VALUE, hInst, NULL);
        btn_edit_value = CreateWindow(WC_BUTTON, L"Edit value", WS_CHILD | WS_VISIBLE, rcClient.left + 320, rcClient.top + 360, 120, 60, hWnd, (HMENU)ID_BUTTON_EDIT_VALUE, hInst, NULL);
        editName = CreateWindow(WC_EDIT, L"Enter key name", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
            rcClient.left + 10, rcClient.bottom - 110, 300, 20, hWnd, (HMENU)ID_EDIT_NAME, hInst, NULL);
        editData1 = CreateWindow(L"edit", L"Enter key/value data", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
            rcClient.left + 10, rcClient.bottom - 80, 300, 20, hWnd, (HMENU)ID_EDIT_DATA1, hInst, NULL);

        editData2 = CreateWindow(L"edit", L"Enter new value data", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
            rcClient.left + 10, rcClient.bottom - 50, 300, 20, hWnd, (HMENU)ID_EDIT_DATA2, hInst, NULL);

        InitCommonControls();
        // Create Tree-View control
        hTreeView = CreateWindowEx(0, WC_TREEVIEW, L"Registry Tree View",
            WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
            10, 10, 300, 410, hWnd, NULL, hInst, NULL);
        LoadRegistry();

    }
    break;
    case WM_COMMAND:
        {
        //EN_KILLFOCUS
            if (LOWORD(wParam) == ID_EDIT_NAME)
            {
                if(HIWORD(wParam) == EN_SETFOCUS)
                    SetWindowText(editName, L"");
                else if (HIWORD(wParam) == EN_KILLFOCUS)
                {
                    WCHAR buffer[256];
                    GetWindowText(editName, buffer, 256);
                    if (_tcslen(buffer) == 0)
                    {
                        SetWindowText(editName, L"Enter key name");
                    }
                }

            }
            else if (LOWORD(wParam) == ID_EDIT_DATA1)
            {
                if (HIWORD(wParam) == EN_SETFOCUS)
                    SetWindowText(editData1, L"");
                else if (HIWORD(wParam) == EN_KILLFOCUS)
                {
                    WCHAR buffer[256];
                    GetWindowText(editData1, buffer, 256);
                    if (_tcslen(buffer) == 0)
                    {
                        SetWindowText(editData1, L"Enter key/value data");
                    }
                }

            }
            else if (LOWORD(wParam) == ID_EDIT_DATA2)
            {
                if (HIWORD(wParam) == EN_SETFOCUS)
                    SetWindowText(editData2, L"");
                else if (HIWORD(wParam) == EN_KILLFOCUS)
                {
                    WCHAR buffer[256];
                    GetWindowText(editData2, buffer, 256);
                    if (_tcslen(buffer) == 0)
                    {
                        SetWindowText(editData2, L"Enter new value data");
                    }
                }

            }

            int wmId = LOWORD(wParam);
            WCHAR nameBuffer[256];
            WCHAR dataBuffer[256];
            WCHAR dataBuffer2[256];
            std::wstring buffer;
            std::wstring keyNameBuffer;
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_BUTTON_CREATE_KEY:
            {
                GetWindowText(editName, nameBuffer, 256);

                if (GetSelectedItemFullPath(&buffer, &keyNameBuffer)) {
                    if (!buffer.empty()) buffer.append(L"\\");
                    buffer.append(keyNameBuffer);
                    AddRegistryKey(nameBuffer, buffer.c_str());
                }
                else AddRegistryKey(nameBuffer);

            }
            break;
            case ID_BUTTON_DELETE_KEY:
            {
                GetWindowText(editName, nameBuffer, 256);

                if (GetSelectedItemFullPath(&buffer, &keyNameBuffer)) {

                    DeleteRegistryKey(keyNameBuffer.c_str(), buffer.c_str());
                }
                else DeleteRegistryKey(nameBuffer);
            }
                break;
            case ID_BUTTON_EDIT_KEY:
            {
                GetWindowText(editName, nameBuffer, 256);
                GetWindowText(editData1, dataBuffer, 256);

                if (GetSelectedItemFullPath(&buffer, &keyNameBuffer)) {

                    EditRegistryKey(keyNameBuffer.c_str(), dataBuffer, buffer.c_str());
                }
                else EditRegistryKey(nameBuffer, dataBuffer);
            }
                break;
            case ID_BUTTON_ADD_VALUE:
            {
                GetWindowText(editName, nameBuffer, 256);
                GetWindowText(editData1, dataBuffer, 256);
                GetWindowText(editData2, dataBuffer2, 256);

                if (GetSelectedItemFullPath(&buffer, &keyNameBuffer)) {

                    AddRegistryValue(keyNameBuffer.c_str(), dataBuffer, dataBuffer2, buffer.c_str());
                }
                else AddRegistryValue(nameBuffer, dataBuffer, dataBuffer2);
            }
                break;
            case ID_BUTTON_DELETE_VALUE:
            {
                GetWindowText(editName, nameBuffer, 256);
                GetWindowText(editData1, dataBuffer, 256);
                GetWindowText(editData2, dataBuffer2, 256);

                if (GetSelectedItemFullPath(&buffer, &keyNameBuffer)) {

                    DeleteRegistryValue(keyNameBuffer.substr(0, keyNameBuffer.find(L" Type: ")).c_str(), buffer.c_str());
                }
                else DeleteRegistryValue(nameBuffer, dataBuffer);

            }
                break;
            case ID_BUTTON_EDIT_VALUE:
            {
                std::wstring valueKeyNameBuffer;
                GetWindowText(editName, nameBuffer, 256);
                GetWindowText(editData1, dataBuffer, 256);
                GetWindowText(editData2, dataBuffer2, 256);

                if (GetSelectedItemFullPath(&buffer, &keyNameBuffer, &valueKeyNameBuffer)) {

                    EditRegistryValue(valueKeyNameBuffer.c_str(), keyNameBuffer.substr(0, keyNameBuffer.find(L" Type: ")).c_str(), dataBuffer2, buffer.c_str());
                }
                else EditRegistryValue(nameBuffer, dataBuffer, dataBuffer2);
            }
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
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
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
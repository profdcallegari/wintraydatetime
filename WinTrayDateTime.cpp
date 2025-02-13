/*
 * WinTrayDateTime.cpp
 *
 *  Created on: 12 de fev de 2025
 *  Daniel Callegari
 * 
 *  Aplicativo para Windows que exibe um ícone na bandeja do sistema 
 *  e permite copiar a data e hora atuais para a área de transferência.
 * 
 */


#include <windows.h>
#include <shellapi.h>
#include <string>
#include <ctime>
#include <iostream>

#define ID_TRAY_ICON 1001
#define ID_COPY_DATE 1002
#define ID_COPY_TIME 1003
#define ID_COPY_DATETIME 1004
#define ID_EXIT      1099

NOTIFYICONDATA nid;
HWND hwnd;

void CopyToClipboard(const std::wstring& text) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
        if (hMem) {
            memcpy(GlobalLock(hMem), text.c_str(), (text.size() + 1) * sizeof(wchar_t));
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        } else {
            std::wcerr << L"Failed to allocate global memory." << std::endl;
        }
        CloseClipboard();
    } else {
        std::wcerr << L"Failed to open clipboard." << std::endl;
    }
}

void ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_COPY_DATE, L"Copiar Data");
    AppendMenu(hMenu, MF_STRING, ID_COPY_TIME, L"Copiar Hora");
	AppendMenu(hMenu, MF_STRING, ID_COPY_DATETIME, L"Copiar Data e Hora");
    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenu(hMenu, MF_STRING, ID_EXIT, L"Sair");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(hMenu);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_EXIT) {
                Shell_NotifyIcon(NIM_DELETE, &nid);
                PostQuitMessage(0);
            }
            else if (LOWORD(wParam) == ID_COPY_DATE || LOWORD(wParam) == ID_COPY_TIME || LOWORD(wParam) == ID_COPY_DATETIME) {
                time_t now = time(nullptr);
                struct tm t;
                localtime_s(&t, &now);
                wchar_t buffer[100];
                if (LOWORD(wParam) == ID_COPY_DATE)
                    wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%d/%m/%Y", &t);
				else if (LOWORD(wParam) == ID_COPY_TIME)
                    wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%H:%M:%S", &t);
                else
					wcsftime(buffer, sizeof(buffer) / sizeof(wchar_t), L"%d/%m/%Y %H:%M:%S", &t);

                CopyToClipboard(buffer);
            }
            break;

        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;

        case WM_USER + 1:
            if (lParam == WM_RBUTTONUP) {
                ShowContextMenu(hwnd);
            }
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"TrayIconClass", nullptr };
    if (!RegisterClassEx(&wc)) {
        OutputDebugString(L"Failed to register window class\n");
        return 1;
    }

    hwnd = CreateWindowEx(0, L"TrayIconClass", L"", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) {
        OutputDebugString(L"Failed to create window\n");
        return 1;
    }

    //OutputDebugString(L"Iniciando\n");

    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(nullptr, IDI_INFORMATION);
    wcscpy_s(nid.szTip, L"Copia Data/Hora para a área de transferência");
    if (!Shell_NotifyIcon(NIM_ADD, &nid)) {
        OutputDebugString(L"Failed to add tray icon\n");
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterClass(L"TrayIconClass", hInstance);
    return 0;
}

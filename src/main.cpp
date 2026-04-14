#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <thread>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "controller.hpp"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

using json = nlohmann::json;

// Глобальні змінні для керування вікном та сервером
NOTIFYICONDATA nid = {};
bool keep_running = true;
httplib::Server svr;

// Обробка повідомлень від іконки в треї
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit Edytor Studio Link");
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
    } else if (uMsg == WM_COMMAND) {
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            keep_running = false;
            svr.stop();
            PostQuitMessage(0);
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main() {
    // 1. ЗАХИСТ ВІД ПОВТОРНОГО ЗАПУСКУ
    HANDLE hMutex = CreateMutex(NULL, TRUE, "Global\\EdytorStudioControllerMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, "Application is already running!", "Controller Link", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // 2. СТВОРЕННЯ ПРИХОВАНОГО ВІКНА ДЛЯ ТРЕЮ
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "TrayWindowClass";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, "TrayWindowClass", "TrayWindow", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);

    // 3. ДОДАВАННЯ ІКОНКИ В ТРЕЙ
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Можна змінити на власну .ico
    strcpy(nid.szTip, "Edytor Studio Controller Link");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // 4. ЗАПУСК КОНТРОЛЕРА ТА СЕРВЕРА В ОКРЕМОМУ ПОТОЦІ
    PSController controller;
    controller.connect();

    std::thread server_thread([&]() {
        svr.Post("/control", [&](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            try {
                auto d = json::parse(req.body);
                controller.update(d["r"], d["g"], d["b"], d["v1"], d["v2"]);
                res.set_content("{\"status\":\"ok\"}", "application/json");
            } catch (...) {}
        });
        svr.listen("127.0.0.1", 8080);
    });

    // 5. ЦИКЛ ПОВІДОМЛЕНЬ WINDOWS
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && keep_running) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Очищення
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (server_thread.joinable()) server_thread.detach(); // Зупиняємо сервер силоміць
    if (hMutex) ReleaseMutex(hMutex);
    
    return 0;
}

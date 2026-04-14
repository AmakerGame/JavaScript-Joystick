#define WIN32_LEAN_AND_MEAN // КРИТИЧНО: Виправляє помилку зі скріншота
#include <windows.h>
#include <winsock2.h>       // Обов'язково перед іншими заголовками
#include <shellapi.h>
#include <thread>
#include <iostream>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "controller.hpp"

#pragma comment(lib, "ws2_32.lib") // Додає бібліотеку сокетів

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

using json = nlohmann::json;

NOTIFYICONDATA nid = {};
bool keep_running = true;
httplib::Server svr;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TRAYICON) {
        if (lParam == WM_RBUTTONUP) {
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ЗАХИСТ ВІД ПОВТОРНОГО ЗАПУСКУ
    HANDLE hMutex = CreateMutex(NULL, TRUE, "Global\\EdytorStudioControllerMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, "Application is already running!", "Controller Link", MB_OK | MB_ICONEXCLAMATION);
        return 0;
    }

    // ТРЕЙ ТА ВІКНО
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TrayClass";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, "TrayClass", "Tray", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(nid.szTip, "Edytor Studio Controller Link");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // КОНТРОЛЕР
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

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (hMutex) ReleaseMutex(hMutex);
    return 0;
}

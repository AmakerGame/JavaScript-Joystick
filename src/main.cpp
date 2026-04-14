#ifdef _WIN32
    #define _WIN32_WINNT 0x0601 // Ціль — Windows 7
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <shellapi.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <signal.h>
#endif

#include <thread>
#include <iostream>
#include "httplib.h"
#include "nlohmann/json.hpp"
#include "controller.hpp"

using json = nlohmann::json;
httplib::Server svr;
bool keep_running = true;

#ifdef _WIN32
// --- WINDOWS SPECIFIC: Tray & Mutex ---
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
NOTIFYICONDATA nid = {};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_TRAYICON && lParam == WM_RBUTTONUP) {
        POINT pt; GetCursorPos(&pt);
        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Exit Edytor Studio Link");
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
    } else if (uMsg == WM_COMMAND && LOWORD(wParam) == ID_TRAY_EXIT) {
        keep_running = false;
        svr.stop();
        PostQuitMessage(0);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

void run_server(PSController& controller) {
    svr.Post("/control", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        try {
            auto d = json::parse(req.body);
            controller.update(d["r"], d["g"], d["b"], d["v1"], d["v2"]);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } catch (...) { res.status = 400; }
    });
    std::cout << "Server started on http://127.0.0.1:8080" << std::endl;
    svr.listen("127.0.0.1", 8080);
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HANDLE hMutex = CreateMutex(NULL, TRUE, "Global\\EdytorStudioControllerMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, "Already running!", "Error", MB_OK);
        return 0;
    }

    WNDCLASS wc = {0}; wc.lpfnWndProc = WindowProc; wc.hInstance = hInstance; wc.lpszClassName = "TrayClass";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowEx(0, "TrayClass", "Tray", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    nid.cbSize = sizeof(NOTIFYICONDATA); nid.hWnd = hwnd; nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(nid.szTip, "Edytor Studio Controller Link");
    Shell_NotifyIcon(NIM_ADD, &nid);

    PSController controller;
    controller.connect();
    std::thread srv_thread(run_server, std::ref(controller));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    return 0;
}
#else
// --- LINUX/MAC SPECIFIC: Console Mode ---
int main() {
    PSController controller;
    if (controller.connect()) std::cout << "Controller Connected!" << std::endl;
    run_server(controller);
    return 0;
}
#endif

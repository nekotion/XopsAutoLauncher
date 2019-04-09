#include <Windows.h>
#include <vector>
#include "Shlwapi.h"

#define CONFIG_INI "XopsAutoLauncher.ini"

namespace controlId {
    const int addressEdit = 0x3EA;
    const int portEdit = 0x3EB;
    const int startButton = 0x3E9;
};

HWND getParentHWndByPID(DWORD pid);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    if (__argc == 2 && !strcmp(__argv[1], "-v")) {
        MessageBox(nullptr, "XOPS Auto Launcher ver0.1", "Info", MB_ICONINFORMATION);
        return 0;
    }

    CHAR LauncherExePath[MAX_PATH];
    GetModuleFileName(nullptr, LauncherExePath, MAX_PATH);
    
    std::string dirXopsPath(LauncherExePath);
    dirXopsPath = dirXopsPath.substr(0, dirXopsPath.find_last_of('\\') + 1);
    //dirXopsPath = R"(D:\tmp\Games\xopsolt19f2\)";

    SetCurrentDirectory(dirXopsPath.c_str());

    // get config
    if (!PathFileExists(CONFIG_INI)) {
        MessageBox(nullptr, CONFIG_INI " not found", "Error", MB_ICONERROR);
        return 1;
    }

    std::string iniPath = dirXopsPath + CONFIG_INI;

    CHAR xopsExeFile[MAX_PATH];
    GetPrivateProfileString("Config", "exe_name", "", xopsExeFile, MAX_PATH, iniPath.c_str());
    if (strcmp(xopsExeFile, "") == 0) {
        MessageBox(nullptr, R"(key "exe_name" not found in )" CONFIG_INI, "Error", MB_ICONERROR);
        return 1;
    }

    CHAR serverAddr[128];
    GetPrivateProfileString("Config", "server_addr", "", serverAddr, MAX_PATH, iniPath.c_str());
    if (strcmp(serverAddr, "") == 0) {
        MessageBox(nullptr, R"(key "server_addr" not found in )" CONFIG_INI, "Error", MB_ICONERROR);
        return 1;
    }

    CHAR serverPort[6];
    GetPrivateProfileString("Config", "server_port", "", serverPort, 6, iniPath.c_str());
    if (strcmp(serverPort, "") == 0) {
        MessageBox(nullptr, R"(key "serverPortStr" not found in )" CONFIG_INI, "Error", MB_ICONERROR);
        return 1;
    }

    // xopsexe
    if (strstr(xopsExeFile, "\\") != nullptr || strstr(xopsExeFile, "/")) {
        MessageBox(nullptr, "bad xops exe filename!", "Error", MB_ICONERROR);
        return 1;
    }
    
    // exe exists?
    if (!PathFileExists(xopsExeFile)) {
        MessageBox(nullptr, "xops exe not found!", "Error", MB_ICONERROR);
        return 1;
    }

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    CreateProcess(xopsExeFile, nullptr, nullptr, nullptr, false, 0, nullptr, nullptr, &si, &pi);
    
    const auto pid = pi.dwProcessId;
    HWND hWnd = nullptr;

    // retry until the window is shown ٩( ᐛ )و
    int cntRetryGetHandle = 0;
    while ((hWnd = getParentHWndByPID(pid)) == nullptr && cntRetryGetHandle < 100) {
        Sleep(50);
        cntRetryGetHandle++;
    };
    
    if (!hWnd) {
        MessageBox(nullptr, "Failed to get window handle!", "Error", MB_ICONERROR);
        return 1;
    }

    auto addressEditCtrl = GetDlgItem(hWnd, controlId::addressEdit);
    SendMessage(addressEditCtrl, WM_SETTEXT, 0, (LPARAM)serverAddr);

    auto portEditCtrl = GetDlgItem(hWnd, controlId::portEdit);
    SendMessage(portEditCtrl, WM_SETTEXT, 0, (LPARAM)serverPort);

    auto startButtonCtrl = GetDlgItem(hWnd, controlId::startButton);
    PostMessage(startButtonCtrl, BM_CLICK, 0, 0);
    
    return 0;
}

HWND getParentHWndByPID(DWORD pid)
{
    HWND hWnd = GetTopWindow(nullptr);
    do {
        if (GetWindowLong(hWnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible(hWnd)) {
            continue;
        }
        DWORD ProcessID;
        GetWindowThreadProcessId(hWnd, &ProcessID);
        if (pid == ProcessID) {
            return hWnd;
        }
    } while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != nullptr);

    return nullptr;
}

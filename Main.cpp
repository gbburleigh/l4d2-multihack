#include <iostream>
#include <tchar.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <random>
#include <WinUser.h>
#include "hotkeys.h"

using namespace std;

int result = SetProcessDPIAware();

struct initResult {
    HANDLE gameProc = NULL;
    DWORD clientBaseAddr = NULL;
    BOOL successful = TRUE;
    HWND gameHandle = NULL;
    DWORD hThread = NULL;

    initResult() {
        ;
    }

    initResult(HANDLE g, DWORD c, HWND gH, BOOL s, DWORD h) {
        gameProc = g;
        clientBaseAddr = c;
        successful = s;
        gameHandle = gH;
        hThread = h;
    }
};

initResult i;
POINT pt;
BOOL __exit__ = FALSE;
DWORD autoFireToggleCount = 0;
BOOL autoFireToggled = FALSE;
INPUT input{ 0 };
DWORD playerBasePtr = 0;
DWORD mFlags = 0;
BOOL autoFireInternalToggle = FALSE;
BOOL toggled = FALSE;
BOOL glowToggled = FALSE;
BOOL vomitToggled = FALSE;
DWORD toggleCount = 0;
DWORD glowToggleCount = 0;
DWORD vomitToggleCount = 0;
DWORD fogToggleCount = 0;
BOOL fogToggled = FALSE;
DWORD MAX_INPUT_DELAY = 50000;
LONG pX;
LONG pY;

BOOL patch_glow(HANDLE gameProc, BOOL mode, const DWORD clientBaseAddr) {
    BYTE nopedBytes[] = { 0x90, 0x90 };
    BYTE patchedBytes[] = { 0x74, 0x6F };
    unsigned long		Protection;
    char* addr = (char*)clientBaseAddr + 0x257732;
    BOOL result = FALSE;

    if (mode) {
        result = WriteProcessMemory(gameProc, addr, &nopedBytes, sizeof(nopedBytes), NULL);
    }
    else {
        result = WriteProcessMemory(gameProc, addr, &patchedBytes, sizeof(patchedBytes), NULL);
    }

    return result;
}

BOOL disable_boomer_vomit(HANDLE gameProc, BOOL mode, const DWORD clientBaseAddr) {
    BYTE nopedBytes[] = { 0x90, 0x90 };
    BYTE patchedBytes[] = { 0x74, 0x5C};

    unsigned long		Protection;
    char* addr = (char*)clientBaseAddr + 0x25D6F7;
    BOOL result = FALSE;

    if (mode) {
        result = WriteProcessMemory(gameProc, addr, &nopedBytes, sizeof(nopedBytes), NULL);
    }
    else {
        result = WriteProcessMemory(gameProc, addr, &patchedBytes, sizeof(patchedBytes), NULL);
    }

    return result;
}

BOOL disable_fog(HANDLE gameProc, BOOL mode, const DWORD clientBaseAddr) {
    BYTE zeroBytes[] = { 0x00, 0x00, 0x00 };
    BYTE patchedBytes[] = { 0x80, 0x3F, 0x01 };

    char* overwriteAddr = (char*)clientBaseAddr + 0x7A272F;
    char* enableAddr = (char*)clientBaseAddr + 0x7A284F;
    BOOL result = FALSE;

    if (mode) {
        result = WriteProcessMemory(gameProc, overwriteAddr, &patchedBytes, sizeof(zeroBytes), NULL);
        result = WriteProcessMemory(gameProc, enableAddr, &zeroBytes, sizeof(zeroBytes), NULL);
    }
    else {
        result = WriteProcessMemory(gameProc, overwriteAddr, &zeroBytes, sizeof(zeroBytes), NULL);
        result = WriteProcessMemory(gameProc, enableAddr, &patchedBytes, sizeof(zeroBytes), NULL);
    }

    return result;
}

DWORD get_client_dll_base_address(HANDLE hSnapshot) {
    MODULEENTRY32 module_entry32;
    module_entry32.dwSize = sizeof(module_entry32);

    BOOL was_copied = Module32First(hSnapshot, &module_entry32);

    while (was_copied) {
        if (_tcscmp(TEXT("client.dll"), module_entry32.szModule) == 0) {
            return (DWORD)module_entry32.modBaseAddr;
        }

        was_copied = Module32Next(hSnapshot, &module_entry32);
    }

    return 0;
}

initResult initialize() {
    initResult i = initResult();
    HWND gameHwnd = NULL;
    int count = 0;
    while ((gameHwnd = FindWindow(NULL, TEXT("Left 4 Dead 2 - Direct3D 9"))) == NULL) {
        count++;
        Sleep(1000);
        if (count > 99) {
            cout << "Failed to locate game window" << "\n";
            i.successful = FALSE;
            return i;
        }
    }

    DWORD gamePid = 0;
    DWORD hThread = GetWindowThreadProcessId(gameHwnd, &gamePid);

    HANDLE gameProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, gamePid);
    if (gameProc == NULL || gamePid == 0) {
        cout << "Failed to open process handle" << "\n";
        i.successful = FALSE;
        return i;
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, gamePid);
    if (hSnap == NULL) {
        cout << "Failed to create snapshot" << "\n";
        i.successful = FALSE;
        return i;
    }

    const DWORD clientBaseAddr = get_client_dll_base_address(hSnap);
    if (clientBaseAddr == 0) {
        cout << "Failed to find base address for client.dll" << "\n";
        i.successful = FALSE;
        return i;
    }

    CloseHandle(hSnap);
    i.clientBaseAddr = clientBaseAddr;
    i.gameProc = gameProc;
    i.gameHandle = gameHwnd;
    i.hThread = hThread;

    return i;
}

void mainCheatThread() {
    while (TRUE) {
        if (toggled) {
            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
                ReadProcessMemory(i.gameProc, (char*)(i.clientBaseAddr + 0x6E1008), &mFlags, sizeof(mFlags), NULL);
                if (mFlags == 0) {
                    SendMessage(i.gameHandle, WM_KEYDOWN, VK_SPACE, 0x390000);
                }
                else {
                    SendMessage(i.gameHandle, WM_KEYUP, VK_SPACE, 0x390000);
                }
            }
            Sleep(0);
        }
        if (__exit__) {
            return;
        }
    }
}

void autoFireThread() {
    while (TRUE) {
        if (autoFireInternalToggle) {
            while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                LRESULT lr = SendMessage(i.gameHandle, WM_LBUTTONDOWN, MK_LBUTTON,0);
                Sleep(50);
            }
        }
        if (__exit__) {
            return;
        }
    }
}

void handleExitThread() {
    while (TRUE) {
        if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000) {
            __exit__ = TRUE;
            cout << "Exiting program..." << "\n";
        }
        if (__exit__) {
            return;
        }
    }
}

void handleCursorThread() {
    while (TRUE) {
        if (GetCursorPos(&pt)) {
            pX = pt.x;
            pY = pt.y;
        }
        if (__exit__) {
            return;
        }
    }
}

void handleInputThread() {
    while (TRUE) {
        if (autoFireToggled && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
            autoFireInternalToggle = TRUE;
        }
        else {
            autoFireInternalToggle = FALSE;
        }
        if (GetAsyncKeyState(AUTO_HOP_TOGGLE) & 0x8000) {
            if (toggleCount > MAX_INPUT_DELAY) {
                if (toggled) {
                    cout << "[AutoHop] Toggled off" << "\n";
                    toggled = FALSE;
                }
                else {
                    cout << "[AutoHop] Toggled on" << "\n";
                    toggled = TRUE;
                }
                toggleCount = 0;
            }
        }
        if (GetAsyncKeyState(GLOW_HACK_TOGGLE) & 0x8000) {
            if (glowToggleCount > MAX_INPUT_DELAY) {
                if (glowToggled) {
                    cout << "[GlowHack] Toggled off" << "\n";
                    glowToggled = FALSE;
                    patch_glow(i.gameProc, glowToggled, i.clientBaseAddr);
                }
                else {
                    cout << "[GlowHack] Toggled on" << "\n";
                    glowToggled = TRUE;
                    patch_glow(i.gameProc, glowToggled, i.clientBaseAddr);
                }
                glowToggleCount = 0;
            }
        }
        if (GetAsyncKeyState(ANTI_VOMIT_TOGGLE) & 0x8000) {
            if (vomitToggleCount > MAX_INPUT_DELAY) {
                if (vomitToggled) {
                    cout << "[VomitHack] Toggled off" << "\n";
                    vomitToggled = FALSE;
                    disable_boomer_vomit(i.gameProc, vomitToggled, i.clientBaseAddr);
                }
                else {
                    cout << "[VomitHack] Toggled on" << "\n";
                    vomitToggled = TRUE;
                    disable_boomer_vomit(i.gameProc, vomitToggled, i.clientBaseAddr);
                }
                vomitToggleCount = 0;
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000) {
            if (autoFireToggleCount > MAX_INPUT_DELAY) {
                if (autoFireToggled) {
                    cout << "[AutoFire] Toggled off" << "\n";
                    autoFireToggled = FALSE;
                }
                else {
                    cout << "[AutoFire] Toggled on" << "\n";
                    autoFireToggled = TRUE;
                }
                autoFireToggleCount = 0;
            }
        }
        if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000) {
            if (fogToggleCount > MAX_INPUT_DELAY) {
                if (fogToggled) {
                    cout << "[NoFog] Toggled off" << "\n";
                    //NOTE: This is unsafe for official servers right now!
                    //disable_fog(i.gameProc, !fogToggled, i.clientBaseAddr);
                    fogToggled = FALSE;
                }
                else {
                    cout << "[NoFog] Toggled on" << "\n";
                    //disable_fog(i.gameProc, !fogToggled, i.clientBaseAddr);
                    fogToggled = TRUE;
                }
                fogToggleCount = 0;
            }
        }
        if (__exit__) {
            return;
        }
        toggleCount++;
        glowToggleCount++;
        vomitToggleCount++;
        autoFireToggleCount++;
        fogToggleCount++;
    }
}

int main(int argc, char* argv[]) {
    cout << "[D R A I N H A C K v 0 . 1 . 5 ]" << '\n';
    cout << "Starting Mumble Overlay" << "\n";

    ShellExecute(NULL, "open", "C:\\Program Files (x86)\\Mumble\\mumble.exe", NULL, NULL, SW_SHOWDEFAULT);
    if (GetLastError() != 0) {
        cout << "Failed to launch mumble.exe" << "\n";
    }
    Sleep(2000);
    ShellExecute(NULL, "open", "C:\\Program Files (x86)\\Steam\\steam.exe", "-applaunch 550", NULL, SW_SHOWDEFAULT);
    if (GetLastError() != 0) {
        cout << GetLastError() << "\n";
        cout << "Failed to launch left4dead2.exe" << "\n";
        return EXIT_FAILURE;
    }
    Sleep(10000);
    input.type = INPUT_MOUSE;
    i = initialize();

    if (!i.successful) {
        cout << "Failed to initialize" << "\n";
        return EXIT_FAILURE;
    }
    else {
        cout << "Mounted process successfully" << "\n";
    }

    SendMessage(i.gameHandle, WM_KEYDOWN, VK_INSERT, 0x390000);
    SendMessage(i.gameHandle, WM_KEYUP, VK_INSERT, 0x390000);
    
    thread t1(mainCheatThread);
    thread t2(autoFireThread);
    thread t3(handleExitThread);
    thread t4(handleInputThread);
    thread t5(handleCursorThread);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}
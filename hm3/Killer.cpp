#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>

void HumanReadableErrorExit() {
    DWORD dw = GetLastError();
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

     printf( ("Error %d: %s\n"), dw, (LPCTSTR)lpMsgBuf);
    system("pause");

    LocalFree(lpMsgBuf);
    ExitProcess(dw);
}

DWORD FindProcessId(const TCHAR* processName) {
    DWORD pid = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if ( strcmp(pe32.szExeFile, processName) == 0) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return pid;
}

int main() {
    const TCHAR* targetProcessName =  ("filter.exe");

    DWORD filterPid = FindProcessId(targetProcessName);

    if (filterPid != 0) {
         printf( ("%s PID: %d\n"), targetProcessName, filterPid);

        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, filterPid);

        if (hProcess == NULL) {
             printf( ("Failed to open process\n"));
        }
        else {
            if (TerminateProcess(hProcess, 0)) {
                 printf( ("%s terminated successfully\n"), targetProcessName);
            }
            else {
                 printf( ("Failed to terminate %s\n"), targetProcessName);
            }
            CloseHandle(hProcess);
        }
    }
    else {
         printf( ("%s is not running\n"), targetProcessName);
    }

    return 0;
}

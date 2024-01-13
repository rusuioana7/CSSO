#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <psapi.h>

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

void CreateDirectoryHierarchy(const char* path) {
    if (!CreateDirectory(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        HumanReadableErrorExit();
}

HANDLE CreateFiles(const char* fileName) {
    HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        HumanReadableErrorExit();
    return hFile;
}

void WriteInFile(HANDLE hFile, const char* content) {
    DWORD bytesWritten;

    if (!WriteFile(hFile, content, (DWORD)strlen(content), &bytesWritten, NULL)) {
        HumanReadableErrorExit();
    }
}

void EnumerateInFiles(HANDLE hProcessFile, HANDLE hThreadFile, HANDLE hModuleFile, int& processCount, int& threadCount, int& moduleCount) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
        HumanReadableErrorExit();

    if (Process32First(hProcessSnap, &pe32)) {
        do {
            char processInfo[256];
            sprintf_s(processInfo, sizeof(processInfo), "ParentProcessId: %lu, ProcessId: %lu, SzExeFile: %s\n", pe32.th32ParentProcessID, pe32.th32ProcessID, pe32.szExeFile);
            WriteInFile(hProcessFile, processInfo);

            processCount++;

            HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pe32.th32ProcessID);

            if (hThreadSnap == INVALID_HANDLE_VALUE)
                HumanReadableErrorExit();

            THREADENTRY32 te32;
            te32.dwSize = sizeof(THREADENTRY32);

            int threadLineCount = 0;

            if (Thread32First(hThreadSnap, &te32)) {
                do {
                    char threadInfo[128];
                    sprintf_s(threadInfo, sizeof(threadInfo), "ThreadId: %lu, OwnerProcessId: %lu\n", te32.th32ThreadID, pe32.th32ProcessID);
                    WriteInFile(hThreadFile, threadInfo);

                    threadLineCount++;
                    threadCount++;
                } while (Thread32Next(hThreadSnap, &te32));
            }

            char threadLineCountStr[256];
            sprintf_s(threadLineCountStr, sizeof(threadLineCountStr), "Fire: %d", threadLineCount);
            WriteInFile(hThreadFile, threadLineCountStr);

            CloseHandle(hThreadSnap);

            if (pe32.th32ProcessID == GetCurrentProcessId()) {
                MODULEENTRY32 me32;
                me32.dwSize = sizeof(MODULEENTRY32);

                HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe32.th32ProcessID);

                if (hModuleSnap == INVALID_HANDLE_VALUE)
                    HumanReadableErrorExit();

                int moduleLineCount = 0;

                if (Module32First(hModuleSnap, &me32)) {
                    do {
                        char moduleInfo[256];
                        sprintf_s(moduleInfo, sizeof(moduleInfo), "ModuleId: 0x%p, ProcessId: %lu, SzModule: %s, SzExePath: %s\n", me32.hModule, pe32.th32ProcessID, me32.szModule, me32.szExePath);
                        WriteInFile(hModuleFile, moduleInfo);

                        moduleLineCount++;
                        moduleCount++;
                    } while (Module32Next(hModuleSnap, &me32));
                }

                char moduleLineCountStr[256];
                sprintf_s(moduleLineCountStr, sizeof(moduleLineCountStr), "Module: %d", moduleLineCount);
                WriteInFile(hModuleFile, moduleLineCountStr);

                CloseHandle(hModuleSnap);
            }
        } while (Process32Next(hProcessSnap, &pe32));
    }

    CloseHandle(hProcessSnap);
}

int main() {
    CreateDirectoryHierarchy("C:\\Facultate");
    CreateDirectoryHierarchy("C:\\Facultate\\CSSO");
    CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare");
    CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare\\Week3");
    CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo");

    char processFilePath[MAX_PATH];
    sprintf_s(processFilePath, sizeof(processFilePath), "C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\procese.txt");
    HANDLE hProcessFile = CreateFiles(processFilePath);

    char threadFilePath[MAX_PATH];
    sprintf_s(threadFilePath, sizeof(threadFilePath), "C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\fire.txt");
    HANDLE hThreadFile = CreateFiles(threadFilePath);

    char moduleFilePath[MAX_PATH];
    sprintf_s(moduleFilePath, sizeof(moduleFilePath), "C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\module_process.txt");
    HANDLE hModuleFile = CreateFiles(moduleFilePath);

    int processCount = 0;
    int threadCount = 0;
    int moduleCount = 0;

    EnumerateInFiles(hProcessFile, hThreadFile, hModuleFile, processCount, threadCount, moduleCount);

    CloseHandle(hProcessFile);
    CloseHandle(hThreadFile);
    CloseHandle(hModuleFile);

    HANDLE hMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int) * 3, "cssoh3basicsync");
    if (hMapping == NULL)
        HumanReadableErrorExit();

    int* pCount = (int*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, 0);
    if (pCount == NULL)
        HumanReadableErrorExit();

    pCount[0] = processCount;
    pCount[1] = threadCount;
    pCount[2] = moduleCount;

    UnmapViewOfFile(pCount);
    CloseHandle(hMapping);

    return 0;
}

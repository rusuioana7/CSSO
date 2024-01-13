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

	printf(("Error %d: %s\n"), dw, (LPCTSTR)lpMsgBuf);
	system("pause");

	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}

struct ThreadData {
	const TCHAR* fileName;
	const TCHAR* substring;
	int result;
};

bool RequestAndEnablePrivilege(const char* privilegeName) {
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return false;

	if (!LookupPrivilegeValue(NULL, privilegeName, &luid))
		return false;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
		return false;

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		return false;

	return true;
}

int SearchString(const TCHAR* fileName, const TCHAR* substring) {
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}

	int lineCount = 0;
	CHAR buffer[1024];
	DWORD bytesRead;

	while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
		CHAR* line = buffer;
		CHAR* end = buffer + bytesRead;

		while (line < end) {
			CHAR* nextLine = strchr(line, '\n');
			if (nextLine == NULL) {
				nextLine = end;
			}

			if (strstr(line, substring) != NULL) {
				lineCount++;
			}

			line = nextLine + 1;
		}
	}

	CloseHandle(hFile);
	return lineCount;
}

DWORD WINAPI ThreadFunction(LPVOID lpParam) {
	ThreadData* data = (ThreadData*)lpParam;
	data->result = SearchString(data->fileName, data->substring);
	return 0;
}

int  main(int argc, CHAR* argv[]) {
	
	if (RequestAndEnablePrivilege(SE_SYSTEM_PROFILE_NAME)) {
		printf("SE_SYSTEM_PROFILE_NAME privilege granted and enabled for filter.exe\n");
	}
	else {
		printf("Failed to request and enable SE_SYSTEM_PROFILE_NAME privilege\n");
	}

	const TCHAR* substring = ("module");

	printf(("Sleeping for 30 minutes...\n"));
	Sleep(30 * 60 * 1000);

	const TCHAR* fileNames[] = {
		 ("C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\procese.txt"),
		 ("C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\fire.txt"),
		 ("C:\\Facultate\\CSSO\\Laboratoare\\Week3\\ProcessInfo\\module_process.txt")
	};

	const int numFiles = sizeof(fileNames) / sizeof(fileNames[0]);
	HANDLE threadHandles[numFiles];
	ThreadData threadData[numFiles];

	for (int i = 0; i < numFiles; i++) {
		threadData[i].fileName = fileNames[i];
		threadData[i].substring = substring;

		threadHandles[i] = CreateThread(NULL, 0, ThreadFunction, &threadData[i], 0, NULL);
		if (threadHandles[i] == NULL) {
			HumanReadableErrorExit();
		}
	}

	WaitForMultipleObjects(numFiles, threadHandles, TRUE, INFINITE);

	for (int i = 0; i < numFiles; i++) {
		printf(("File: %s\nLines containing substring: %d\n"), threadData[i].fileName, threadData[i].result);
	}

	return 0;
}

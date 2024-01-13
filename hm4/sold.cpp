#include <windows.h>
#include <stdio.h>

#define FILE_PATH "C:\\Facultate\\CSSO\\Week4\\Reports\\"

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

	printf("Error %d: %s\n", dw, (LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}

void WriteErrorToLogFile(DWORD shelve_id) {
	const char* errorFile = FILE_PATH "Summary\\errors.txt";
	HANDLE file = CreateFile(
		errorFile,
		FILE_APPEND_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (file != INVALID_HANDLE_VALUE) {
		char errorMsg[100];
		sprintf_s(errorMsg, "S-a încercat vânzarea unui produs de pe un raft <%d> ce nu conține produs\n", shelve_id);
		DWORD bytesWritten = 0;
		WriteFile(file, errorMsg, strlen(errorMsg), &bytesWritten, NULL);
		CloseHandle(file);
	}
	else {
		printf("Failed to open or create errors.txt file.\n");
		HumanReadableErrorExit();
	}
}

void ProcessSoldFiles(const char* soldDirectoryPath) {
	const char* MARKET_SHELVES = "MarketShelves";
	const char* MARKET_VALABILITY = "MarketValability";
	const char* PRODUCT_PRICES = "ProductPrices";
	HANDLE hMapShelves = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MARKET_SHELVES);
	HANDLE hMapValability = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MARKET_VALABILITY);
	HANDLE hMapPrices = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, PRODUCT_PRICES);

	if (hMapShelves != NULL && hMapValability != NULL && hMapPrices != NULL) {
		DWORD* shelves = (DWORD*)MapViewOfFile(hMapShelves, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		DWORD* valability = (DWORD*)MapViewOfFile(hMapValability, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		DWORD* prices = (DWORD*)MapViewOfFile(hMapPrices, FILE_MAP_ALL_ACCESS, 0, 0, 0);

		if (shelves != NULL && valability != NULL && prices != NULL) {
			char soldPath[MAX_PATH];
			sprintf_s(soldPath, "%s\\*.*", soldDirectoryPath);
			WIN32_FIND_DATA fileData;
			HANDLE hFind = FindFirstFile(soldPath, &fileData);

			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						char filePath[MAX_PATH];
						sprintf_s(filePath, "%s\\%s", soldDirectoryPath, fileData.cFileName);

						HANDLE file = CreateFile(
							filePath,
							GENERIC_READ,
							FILE_SHARE_READ,
							NULL,
							OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,
							NULL
						);

						if (file != INVALID_HANDLE_VALUE) {
							int shelve_id;
							DWORD bytesRead;
							ReadFile(file, &shelve_id, sizeof(int), &bytesRead, NULL);
							CloseHandle(file);

							if (shelves[shelve_id] == 0xFFFFFFFF) {
								WriteErrorToLogFile(shelve_id);
								continue;
							}

							int id_produs = shelves[shelve_id];

							if (valability[id_produs] != 0) {
								HANDLE logsFile = CreateFile(
									FILE_PATH "Summary\\logs.txt",
									FILE_APPEND_DATA,
									FILE_SHARE_READ,
									NULL,
									OPEN_ALWAYS,
									FILE_ATTRIBUTE_NORMAL,
									NULL
								);

								if (logsFile != INVALID_HANDLE_VALUE) {
									char logMsg[200];
									sprintf_s(logMsg, "S-a vândut produsul <%d> de pe raftul <%d> cu <%d> zile înainte de a fi donat și cu prețul de <%d>\n", id_produs, shelve_id, valability[id_produs], prices[id_produs]);
									DWORD bytesWritten = 0;
									WriteFile(logsFile, logMsg, strlen(logMsg), &bytesWritten, NULL);
									CloseHandle(logsFile);
								}
								else {
									printf("Failed to open or create logs.txt file.\n");
									HumanReadableErrorExit();
								}

								HANDLE soldFile = CreateFile(
									FILE_PATH "Summary\\sold.txt",
									GENERIC_READ | GENERIC_WRITE,
									FILE_SHARE_READ,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL
								);

								if (soldFile != INVALID_HANDLE_VALUE) {
									int old_value;
									DWORD bytesRead;
									ReadFile(soldFile, &old_value, sizeof(int), &bytesRead, NULL);
									SetFilePointer(soldFile, 0, NULL, FILE_BEGIN);
									old_value += prices[id_produs];
									DWORD bytesWritten = 0;
									WriteFile(soldFile, &old_value, sizeof(int), &bytesWritten, NULL);
									CloseHandle(soldFile);
								}
								else {
									printf("Failed to open sold.txt file.\n");
									HumanReadableErrorExit();
								}

								valability[id_produs] = 0xFFFFFFFF;
								prices[id_produs] = 0xFFFFFFFF;
								shelves[shelve_id] = 0xFFFFFFFF;
							}
						}
						else {
							printf("Failed to open file: %s\n", filePath);
							HumanReadableErrorExit();
						}
					}
				} while (FindNextFile(hFind, &fileData) != 0);

				FindClose(hFind);
			}
			else {
				printf("Failed to open 'sold' directory.\n");
				HumanReadableErrorExit();
			}

			UnmapViewOfFile(shelves);
			UnmapViewOfFile(valability);
			UnmapViewOfFile(prices);
		}
		else {
			printf("Failed to map views of memory-mapped files.\n");
			HumanReadableErrorExit();
		}

		CloseHandle(hMapShelves);
		CloseHandle(hMapValability);
		CloseHandle(hMapPrices);
	}
	else {
		printf("Failed to open memory-mapped files.\n");
		HumanReadableErrorExit();
	}
}

int main() {
	const char* soldDirPath = "C:\\tema4\\sold";
	ProcessSoldFiles(soldDirPath);
	return 0;
}

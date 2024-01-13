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

const int TOTAL_PRODUCTS = 10000;
const int TOTAL_SHELVES = 10000;

void ProcessDonations() {
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
			HANDLE donationsFile = CreateFile(
				(FILE_PATH "Summary\\donations.txt"),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

			HANDLE logsFile = CreateFile(
				(FILE_PATH "Summary\\logs.txt"),
				FILE_APPEND_DATA,
				FILE_SHARE_READ,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

			if (donationsFile != INVALID_HANDLE_VALUE && logsFile != INVALID_HANDLE_VALUE) {
				for (int id_produs = 0; id_produs < TOTAL_PRODUCTS; ++id_produs) {
					int oldValue;
					DWORD bytesRead;
					ReadFile(donationsFile, &oldValue, sizeof(int), &bytesRead, NULL);
					SetFilePointer(donationsFile, 0, NULL, FILE_BEGIN);
					if (valability[id_produs] == 0) {
						oldValue += prices[id_produs];
						WriteFile(donationsFile, &oldValue, sizeof(int), &bytesRead, NULL);

						char logMsg[50];
						sprintf_s(logMsg, "Produsul <%d> a fost donat\n", id_produs);
						DWORD bytesWritten;
						WriteFile(logsFile, logMsg, strlen(logMsg), &bytesWritten, NULL);

						valability[id_produs] = 0xFFFFFFFF;
						prices[id_produs] = 0xFFFFFFFF;

						for (int shelve_id = 0; shelve_id < TOTAL_SHELVES; ++shelve_id) {
							if (shelves[shelve_id] == id_produs) {
								shelves[shelve_id] = 0xFFFFFFFF;
							}
						}
					}
					else if (valability[id_produs] > 0 && valability[id_produs] < 0xFFFFFFFF) {
						valability[id_produs]--;
					}
				}

				CloseHandle(donationsFile);
				CloseHandle(logsFile);
			}
			else {
				printf("Failed to open donations.txt or logs.txt file.\n");
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
	ProcessDonations();
	return 0;
}

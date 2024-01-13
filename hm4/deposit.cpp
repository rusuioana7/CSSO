#include <windows.h>
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

	printf("Error %d: %s\n", dw, (LPCTSTR)lpMsgBuf);
	system("pause");

	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}

void UpdateShelves(DWORD* shelves, DWORD* valability, DWORD* prices, int shelve_id, int id_produs, int expires_in, int product_price) {
	if (shelves[shelve_id] != 0xFFFFFFFF) { //daca raftul e ocupat
		HANDLE hErrorFile = CreateFileA("C:\\Facultate\\CSSO\\Week4\\Reports\\Summary\\errors.txt", FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hErrorFile != INVALID_HANDLE_VALUE) {
			char buffer[256];
			sprintf_s(buffer, sizeof(buffer), "S-a încercat adăugarea produsului %d pe raftul %d care este deja ocupat de %d\n", id_produs, shelve_id, shelves[shelve_id]);
			DWORD bytesWritten;
			WriteFile(hErrorFile, buffer, strlen(buffer), &bytesWritten, NULL);
			CloseHandle(hErrorFile);
		}
		return;
	}
	//daca nu e ocupat
	shelves[shelve_id] = id_produs;
	valability[id_produs] = expires_in;
	prices[id_produs] = product_price;

	HANDLE hLogsFile = CreateFileA("C:\\Facultate\\CSSO\\Week4\\Reports\\logs.txt", FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hLogsFile != INVALID_HANDLE_VALUE) {
		char buffer[256];
		sprintf_s(buffer, sizeof(buffer), "Am adăugat pe raftul %d produsul %d ce are o valabilitate de %d zile și un preț de %d.\n", shelve_id, id_produs, expires_in, product_price);
		DWORD bytesWritten;
		WriteFile(hLogsFile, buffer, strlen(buffer), &bytesWritten, NULL);
		CloseHandle(hLogsFile);
	}
}

void ProcessFiles(const char* directory, DWORD* shelves, DWORD* valability, DWORD* prices) {
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind;

	char searchPath[MAX_PATH];
	sprintf_s(searchPath, MAX_PATH, "%s\\*.*", directory);

	hFind = FindFirstFileA(searchPath, &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		HumanReadableErrorExit();
	}

	do {
		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			char filePath[MAX_PATH];
			sprintf_s(filePath, MAX_PATH, "%s\\%s", directory, findFileData.cFileName);
			HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE) {
				int id_produs, expires_in, shelve_id, product_price;
				DWORD bytesRead;
				char buffer[256];
				while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
					sscanf_s(buffer, "%d %d %d %d", &id_produs, &expires_in, &shelve_id, &product_price);
					if (shelve_id >= 0 && shelve_id < 10000) {
						UpdateShelves(shelves, valability, prices, shelve_id, id_produs, expires_in, product_price);
					}
				}
				CloseHandle(hFile);
			}
			else {
				printf("Could not open file: %s\n", filePath);
			}
		}
	} while (FindNextFileA(hFind, &findFileData) != 0);

	FindClose(hFind);
}

int main() {
	const int NUM_VALUES = 10000;
	const char* MARKET_SHELVES = "MarketShelves";
	const char* MARKET_VALABILITY = "MarketValability";
	const char* PRODUCT_PRICES = "ProductPrices";

	HANDLE hMarketShelves = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MARKET_SHELVES);
	HANDLE hMarketValability = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MARKET_VALABILITY);
	HANDLE hProductPrices = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, PRODUCT_PRICES);

	if (hMarketShelves == NULL || hMarketValability == NULL || hProductPrices == NULL) {
		HumanReadableErrorExit();
	}

	DWORD* shelves = (DWORD*)MapViewOfFile(hMarketShelves, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	DWORD* valability = (DWORD*)MapViewOfFile(hMarketValability, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	DWORD* prices = (DWORD*)MapViewOfFile(hProductPrices, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	if (shelves == NULL || valability == NULL || prices == NULL) {
		HumanReadableErrorExit();
	}

	printf("Mapping 'marketShelves' opened successfully!\n");
	printf("Mapping 'marketValability' opened successfully!\n");
	printf("Mapping 'productPrices' opened successfully!\n");

	const char* depositDirectory = "C:\\tema4\\deposit";
	printf("Accessing directory: %s\n", depositDirectory);
	ProcessFiles(depositDirectory, shelves, valability, prices);

	CloseHandle(hMarketShelves);
	CloseHandle(hMarketValability);
	CloseHandle(hProductPrices);

	return 0;
}

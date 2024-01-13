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

void CreateDirectoryHierarchy(const char* path) {
	if (!CreateDirectory(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		HumanReadableErrorExit();
}


int main() {
	const int NUM_VALUES = 10000;
	const char* FILE_PATH = "C:\\Facultate\\CSSO\\Week4\\Reports\\";
	const char* MARKET_SHELVES = "MarketShelves";
	const char* MARKET_VALABILITY = "MarketValability";
	const char* PRODUCT_PRICES = "ProductPrices";

	//directoarele
	CreateDirectoryHierarchy("C:\\Facultate\\");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Week4\\");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Week4\\Reports\\");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Week4\\Reports\\Summary");

	//file mappings
	HANDLE marketShelves = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_EXECUTE_READWRITE,
		0,
		NUM_VALUES * sizeof(DWORD),
		MARKET_SHELVES
	);

	if (marketShelves == NULL) {
		printf("Failed to create file mapping for marketShelves. Error: %d\n", GetLastError());
	}
	else {
		printf("File mapping for marketShelves created successfully.\n");

		CloseHandle(marketShelves);
	}

	HANDLE marketValability = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_EXECUTE_READWRITE,
		0,
		NUM_VALUES * sizeof(DWORD),
		MARKET_VALABILITY
	);

	if (marketValability == NULL) {
		printf("Failed to create file mapping for marketValability. Error: %d\n", GetLastError());
	}
	else {
		printf("File mapping for marketValability created successfully.\n");

		CloseHandle(marketValability);
	}

	HANDLE productPrices = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_EXECUTE_READWRITE,
		0,
		NUM_VALUES * sizeof(DWORD),
		PRODUCT_PRICES
	);

	if (productPrices == NULL) {
		printf("Failed to create file mapping for productPrices. Error: %d\n", GetLastError());
	}
	else {
		printf("File mapping for productPrices created successfully.\n");
		CloseHandle(productPrices);

		//procesele
		STARTUPINFO si[3];
		PROCESS_INFORMATION pi[3];

		const char* commands[3] = {
			"C:\\Users\\ioana\\facultate\\3-sem1\\csso\\tema4\\Deposit\\x64\\Debug\\Deposit.exe",
			"C:\\Users\\ioana\\facultate\\3-sem1\\csso\\tema4\\Sold\\x64\\Debug\\Sold.exe",
			"C:\\Users\\ioana\\facultate\\3-sem1\\csso\\tema4\\Donation\\x64\\Debug\\Donation.exe"
		};

		for (int i = 0; i < 3; ++i) {
			ZeroMemory(&si[i], sizeof(si[i]));
			si[i].cb = sizeof(si[i]);
			ZeroMemory(&pi[i], sizeof(pi[i]));

			if (!CreateProcess(
				NULL,
				(LPSTR)commands[i],
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&si[i],
				&pi[i])
				) {
				printf("Failed to start process %d. Error: %d\n", i + 1, GetLastError());
			}
		}

		WaitForMultipleObjects(3, &pi[0].hProcess, TRUE, 60000); //timeout 60sec

		for (int i = 0; i < 3; ++i) {
			CloseHandle(pi[i].hProcess);
			CloseHandle(pi[i].hThread);
		}

		//fisierele
		HANDLE hErrorsFile = CreateFile("C:\\Facultate\\CSSO\\Week4\\Reports\\Summary\\errors.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hErrorsFile == INVALID_HANDLE_VALUE) {
			HANDLE hSoldFile = CreateFile("C:\\Facultate\\CSSO\\Week4\\Reports\\Summary\\sold.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			HANDLE hDonationsFile = CreateFile("C:\\Facultate\\CSSO\\Week4\\Reports\\Summary\\donations.txt", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			int soldValue = 0, donationsValue = 0;
			if (hSoldFile != INVALID_HANDLE_VALUE) {
				DWORD bytesRead;
				ReadFile(hSoldFile, &soldValue, sizeof(int), &bytesRead, NULL);
				CloseHandle(hSoldFile);
			}
			if (hDonationsFile != INVALID_HANDLE_VALUE) {
				DWORD bytesRead;
				ReadFile(hDonationsFile, &donationsValue, sizeof(int), &bytesRead, NULL);
				CloseHandle(hDonationsFile);
			}

			printf("Sold Value: %d\nDonations Value: %d\n", soldValue, donationsValue);
		}
		else {
			char buffer[256];
			DWORD bytesRead;
			while (ReadFile(hErrorsFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
				printf("%.*s", bytesRead, buffer);
			}
			CloseHandle(hErrorsFile);
		}

		return 0;
	}
}

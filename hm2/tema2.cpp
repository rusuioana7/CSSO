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

//create the directories
void CreateDirectoryHierarchy(const char* path) {
	if (!CreateDirectory(path, NULL))
		HumanReadableErrorExit();
}

//create the files
HANDLE CreateFiles(const char* fileName) {
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		HumanReadableErrorExit();
	return hFile;
}

//writing in files
void WriteInFile(HANDLE hFile, const char* content) {
	DWORD bytesWritten;

	if (!WriteFile(hFile, content, (DWORD)strlen(content), &bytesWritten, NULL)) {
		HumanReadableErrorExit();
	}
}

//gets info about windows register keys
void QueryRegistryInfo(HKEY hKey, const char* hiveName, HANDLE hFile) {
	DWORD subKeyCount;
	DWORD maxSubKeyLength;
	FILETIME lastWriteTime;
	LSTATUS result = RegQueryInfoKey(hKey, NULL, NULL, NULL, &subKeyCount, &maxSubKeyLength, NULL, NULL, NULL, NULL, NULL, &lastWriteTime);

	if (result != ERROR_SUCCESS) {
		HumanReadableErrorExit();
		return;
	}

	SYSTEMTIME sysTime;
	FileTimeToSystemTime(&lastWriteTime, &sysTime);

	char info[1024];

	sprintf_s(info, "Hive: %s\nSubkey Count: %u\nMax Subkey Length: %u\nLast Write Time: %04d/%02d/%02d %02d:%02d:%02d\n",
		hiveName, subKeyCount, maxSubKeyLength, sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	WriteInFile(hFile, info);
}

// enumerates subkeys of a registry key
void EnumerateRegistryKeys(HKEY hKey, const char* path, const char* outputPath) {
	HKEY hSubKey;
	char subKeyPath[MAX_PATH];
	DWORD subKeyIndex = 0;

	while (RegEnumKey(hKey, subKeyIndex, subKeyPath, MAX_PATH) != ERROR_NO_MORE_ITEMS) {
		char fullSubKeyPath[MAX_PATH];
		sprintf_s(fullSubKeyPath, "%s\\%s", path, subKeyPath);

		char outputFilePath[MAX_PATH];
		sprintf_s(outputFilePath, "%s\\%s.txt", outputPath, subKeyPath);
		HANDLE hFile = CreateFiles(outputFilePath);

		QueryRegistryInfo(hKey, fullSubKeyPath, hFile);

		CloseHandle(hFile);

		if (RegOpenKeyEx(hKey, subKeyPath, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
			EnumerateRegistryKeys(hSubKey, fullSubKeyPath, outputPath);
			RegCloseKey(hSubKey);
		}

		subKeyIndex++;
	}
}

//gets size of a file
DWORD GetFileSize(const char* fileName) {
	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	DWORD fileSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return fileSize;
}

//counts the number of files in a directory
int CountFilesInDirectory(const char* directoryPath) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(directoryPath, &findFileData);
	int fileCount = 0;

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				fileCount++;
			}
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
	}
	else {
		return -1;
	}

	return fileCount;
}

////creates a new registry key
void SetRegistryValues() {
	HKEY hKeyHKCU3;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\CSSO\\Week2", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyHKCU3, NULL) != ERROR_SUCCESS) {
		HumanReadableErrorExit();
	}

	const char* pathValue = "C:\\Facultate\\CSSO\\Laboratoare\\Week3\\InstalledSoftware";
	if (RegSetValueEx(hKeyHKCU3, "PathValue", 0, REG_SZ, (BYTE*)pathValue, (DWORD)(strlen(pathValue) + 1)) != ERROR_SUCCESS) {
		HumanReadableErrorExit();
	}

	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile("C:\\Facultate\\CSSO\\Laboratoare\\Week3\\InstalledSoftware\\*", &findFileData);
	int fileCount = 0;

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			fileCount++;
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
	}

	if (RegSetValueEx(hKeyHKCU3, "FileCount", 0, REG_DWORD, (BYTE*)&fileCount, sizeof(DWORD)) != ERROR_SUCCESS) {
		HumanReadableErrorExit();
	}

	RegCloseKey(hKeyHKCU3);
}

int main() {
	CreateDirectoryHierarchy("C:\\Facultate");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare\\Week2");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\InstalledSoftware");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate");

	HANDLE hFileHKLM = CreateFiles("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKLM.txt");
	HANDLE hFileHKCC = CreateFiles("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKCC.txt");
	HANDLE hFileHKCU = CreateFiles("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKCU.txt");

	HKEY hKeyHKLM, hKeyHKCC, hKeyHKCU;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &hKeyHKLM) != ERROR_SUCCESS)
		HumanReadableErrorExit();

	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, &hKeyHKCC) != ERROR_SUCCESS)
		HumanReadableErrorExit();

	if (RegOpenKeyEx(HKEY_CURRENT_USER, NULL, 0, KEY_READ, &hKeyHKCU) != ERROR_SUCCESS)
		HumanReadableErrorExit();

	QueryRegistryInfo(hKeyHKLM, "HKLM", hFileHKLM);
	QueryRegistryInfo(hKeyHKCC, "HKCC", hFileHKCC);
	QueryRegistryInfo(hKeyHKCU, "HKCU", hFileHKCU);

	RegCloseKey(hKeyHKLM);
	RegCloseKey(hKeyHKCC);
	RegCloseKey(hKeyHKCU);

	CloseHandle(hFileHKLM);
	CloseHandle(hFileHKCC);
	CloseHandle(hFileHKCU);

	DWORD sizeHKLM = GetFileSize("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKLM.txt");
	DWORD sizeHKCC = GetFileSize("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKCC.txt");
	DWORD sizeHKCU = GetFileSize("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKCU.txt");

	HANDLE hFileSumar = CreateFiles("C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\sumar.txt");

	char sumarInfo[1024];
	sprintf_s(sumarInfo, "Path: C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKLM.txt, Size: %u bytes\n", sizeHKLM);
	WriteInFile(hFileSumar, sumarInfo);

	sprintf_s(sumarInfo, "Path: C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKCC.txt, Size: %u bytes\n", sizeHKCC);
	WriteInFile(hFileSumar, sumarInfo);

	sprintf_s(sumarInfo, "Path: C:\\Facultate\\CSSO\\Laboratoare\\Week2\\Rezultate\\HKCU.txt, Size: %u bytes\n", sizeHKCU);
	WriteInFile(hFileSumar, sumarInfo);

	CloseHandle(hFileSumar);

	HKEY hKeyHKCU2;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, NULL, 0, KEY_READ, &hKeyHKCU2) != ERROR_SUCCESS)
		HumanReadableErrorExit();

	EnumerateRegistryKeys(hKeyHKCU2, "HKEY_CURRENT_USER", "C:\\Facultate\\CSSO\\Laboratoare\\Week2\\InstalledSoftware");

	RegCloseKey(hKeyHKCU2);
	SetRegistryValues();


	return 0;
}

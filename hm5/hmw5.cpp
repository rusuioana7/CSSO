#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <stdlib.h> 

#pragma comment(lib, "wininet.lib")

char lastGetResponse[1024];
const char* lastGetURL = NULL;
char* context = NULL;


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

void AppendToFile(const char* filePath, const char* content) {
	HANDLE hFile = CreateFileA(
		filePath,
		FILE_APPEND_DATA,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		HumanReadableErrorExit();
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD bytesWritten;
	WriteFile(hFile, content, strlen(content), &bytesWritten, NULL);
	CloseHandle(hFile);
}

void SaveFileFromURL(const char* url, const char* filePath) {
	HINTERNET hInternet = InternetOpenA("HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		HumanReadableErrorExit();
	}

	HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hConnect) {
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	DWORD bytesRead = 0;
	const DWORD bufferSize = 1024;
	BYTE buffer[bufferSize];

	HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	BOOL bWriteFile = FALSE;
	do {
		bWriteFile = InternetReadFile(hConnect, buffer, bufferSize, &bytesRead);
		if (bytesRead > 0) {
			DWORD bytesWritten;
			WriteFile(hFile, buffer, bytesRead, &bytesWritten, NULL);
		}
	} while (bytesRead > 0 && bWriteFile);

	CloseHandle(hFile);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}

void ProcessGETRequest(const char* url, int k) {
	HINTERNET hInternet = InternetOpenA("HTTPGET", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		HumanReadableErrorExit();
	}

	HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hConnect) {
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	const DWORD bufferSize = 1024;
	BYTE buffer[bufferSize];
	DWORD bytesRead = 0;
	char* response = (char*)malloc(bufferSize);

	if (response == NULL) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	response[0] = '\0';

	do {
		if (!InternetReadFile(hConnect, buffer, bufferSize - 1, &bytesRead)) {
			InternetCloseHandle(hConnect);
			InternetCloseHandle(hInternet);
			free(response);
			HumanReadableErrorExit();
		}

		buffer[bytesRead] = '\0';
		strcat_s(response, bufferSize, (char*)buffer);
	} while (bytesRead > 0);

	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	const char* lastParam = strrchr(url, '/');
	const char* fileName = (lastParam != NULL) ? lastParam + 1 : "response.txt";

	const char* extension = strrchr(fileName, '.');
	if (extension == NULL || strcmp(extension, ".txt") != 0) {
		const char* txtExtension = ".txt";
		char updatedFileName[MAX_PATH];
		strcpy_s(updatedFileName, sizeof(updatedFileName), fileName);
		strcat_s(updatedFileName, sizeof(updatedFileName), txtExtension);
		fileName = updatedFileName;
	}

	const char* downloadPath = "C:\\Facultate\\CSSO\\Week5\\Downloads\\";
	char filePath[MAX_PATH];
	if (k == 1) {
		snprintf(filePath, MAX_PATH, "%s%s", downloadPath, fileName);
	}
	else if (k == 0) snprintf(filePath, MAX_PATH, "%s%s_additional.txt", downloadPath, fileName);
	AppendToFile(filePath, response);

	free(response);
}

void SendPOSTRequest(const char* url, const char* postData) {
	HINTERNET hInternet = InternetOpenA("HTTPPOST", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		HumanReadableErrorExit();
	}

	HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hConnect) {
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	const DWORD postDataSize = strlen(postData);
	DWORD bytesWritten = 0;
	BOOL bWrite = InternetWriteFile(hConnect, postData, postDataSize, &bytesWritten);
	if (!bWrite) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	CloseHandle(hConnect);

	DWORD statusCode;
	DWORD statusCodeSize = sizeof(statusCode);
	HttpQueryInfo(hConnect, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusCodeSize, NULL);


	InternetCloseHandle(hInternet);
}

void ReadFileLineByLine(const char* filePath) {
	HANDLE hFile = CreateFileA(
		filePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		HumanReadableErrorExit();
	}

	DWORD fileSize = GetFileSize(hFile, NULL);
	if (fileSize == INVALID_FILE_SIZE) {
		CloseHandle(hFile);
		HumanReadableErrorExit();
	}

	char* buffer = (char*)malloc(fileSize + 1);
	if (buffer == NULL) {
		CloseHandle(hFile);
		HumanReadableErrorExit();
	}

	DWORD bytesRead;
	if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
		CloseHandle(hFile);
		free(buffer);
		HumanReadableErrorExit();
	}

	buffer[bytesRead] = '\0';
	CloseHandle(hFile);

	char* nextLine = strtok_s(buffer, "\r\n", &context);

	while (nextLine != NULL) {
		printf("Line: %s\n", nextLine);

		if (strstr(nextLine, "dohomework_additional") != NULL) {
			printf("Contains 'dohomework_additional'\n");
			char* colonPos2 = strchr(nextLine, ':');
			if (colonPos2 != NULL) {
				*colonPos2 = '\0';
				char* url = colonPos2 + 1;
				ProcessGETRequest(url, 0);
			}
		}
		else  if (strstr(nextLine, "dohomework/") != NULL) {
			char* colonPos = strchr(nextLine, ':');
			if (colonPos != NULL) {
				*colonPos = '\0';

				if (strcmp(nextLine, "GET") == 0) {
					printf("Method: GET\n");

					char* url = colonPos + 1;
					ProcessGETRequest(url, 1);
				}
				else if (strcmp(nextLine, "POST") == 0) {
					printf("Method: POST\n");
					if (lastGetURL != NULL) {
						char postData[1024];
						snprintf(postData, sizeof(postData), "id=%s&value=%s", "310910401ESL211072", lastGetResponse);

						SendPOSTRequest(lastGetURL, postData);

						lastGetURL = NULL;
					}
				}
			}
		}


		nextLine = strtok_s(NULL, "\r\n", &context);
	}


	free(buffer);
}

void SendEndHomeworkRequest(const char* url, const char* postData) {
	HINTERNET hInternet = InternetOpenA("HTTPPOST", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		HumanReadableErrorExit();
	}

	HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hConnect) {
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	const DWORD postDataSize = strlen(postData);
	DWORD bytesWritten = 0;
	BOOL bWrite = InternetWriteFile(hConnect, postData, postDataSize, &bytesWritten);
	if (!bWrite) {
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		HumanReadableErrorExit();
	}

	CloseHandle(hConnect);

	DWORD statusCode;
	DWORD statusCodeSize = sizeof(statusCode);
	HttpQueryInfo(hConnect, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusCodeSize, NULL);

	InternetCloseHandle(hInternet);
}

int main() {
	const char* url = "http://cssohw.herokuapp.com/assignhomework/310910401ESL211072";
	const char* filePath = "C:\\Facultate\\CSSO\\Week5\\myconfig.txt";

	CreateDirectoryHierarchy("C:\\Facultate");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Week5");
	CreateDirectoryHierarchy("C:\\Facultate\\CSSO\\Week5\\Downloads");

	SaveFileFromURL(url, filePath);

	ReadFileLineByLine(filePath);
	const char* endHomeworkURL = "http://cssohw.herokuapp.com/endhomework";
	char postData[1024];
	snprintf(postData, sizeof(postData), "id=<310910401ESL211072>&total=<total>&get=<get>&post=<post>&size=<size>");

	SendEndHomeworkRequest(endHomeworkURL, postData);


	return 0;
}
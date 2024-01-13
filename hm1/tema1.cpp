#include <Windows.h>
#include <stdio.h>
#include <string.h>

void HumanReadableErrorExit()
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	printf(TEXT("Am urmatoarea eroare: %s\n"), lpMsgBuf);
	system("pause");

	LocalFree(lpMsgBuf);
	ExitProcess(dw);
}

//functie pentru scrierea in sumar.txt
void WriteToSumarFile(const char* filePath, int offset)
{
	HANDLE sumarFileHandle = CreateFile(
		"C:\\Facultate\\CSSO\\Laboratoare\\Week1\\Rezultate\\sumar.txt",
		FILE_APPEND_DATA,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (sumarFileHandle == INVALID_HANDLE_VALUE)
	{
		printf("Eroare la deschiderea sumar.txt\n");
		return;
	}

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "Fisier : %s, Pozitie: %d\n", filePath, offset);
	DWORD bytesWritten;

	SetFilePointer(sumarFileHandle, 0, NULL, FILE_END);
	WriteFile(sumarFileHandle, buffer, (DWORD)strlen(buffer), &bytesWritten, NULL);

	CloseHandle(sumarFileHandle);
}

//functie pentru cautarea cuvantului
void SearchFiles(const char* directory, const char* word)
{
	char searchPath[MAX_PATH];
	snprintf(searchPath, sizeof(searchPath), "%s\\*", directory);

	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(searchPath, &findData);

	if (findHandle == INVALID_HANDLE_VALUE)
	{
		printf("Eroare la cautarea directorului: %s\n", directory);
		return;
	}

	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //directoare
		{

			if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
			{
				char subDirectory[MAX_PATH];
				snprintf(subDirectory, sizeof(subDirectory), "%s\\%s", directory, findData.cFileName);
				
			}
		}
		else //fisiere
		{
			char filePath[MAX_PATH];
			snprintf(filePath, sizeof(filePath), "%s\\%s", directory, findData.cFileName);

			HANDLE fileHandle = CreateFile(
				filePath,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (fileHandle != INVALID_HANDLE_VALUE)
			{
				char buffer[4001];
				DWORD bytesRead;

				if (ReadFile(fileHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
				{
					buffer[bytesRead] = '\0';

					char* position = strstr(buffer, word);
					while (position != NULL)
					{
						int offset = (int)(position - buffer);
						printf("Fisier : %s, Pozitie: %d\n", filePath, offset);
						WriteToSumarFile(filePath, offset);
						position = strstr(position + 1, word);
					}
				}

				CloseHandle(fileHandle);
			}
		}
	} while (FindNextFile(findHandle, &findData));

	FindClose(findHandle);
}

int main()
{
	if (!CreateDirectory("C:\\Facultate", NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		HumanReadableErrorExit();
		return 1;
	}

	if (!CreateDirectory("C:\\Facultate\\CSSO", NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		HumanReadableErrorExit();
		return 1;
	}

	if (!CreateDirectory("C:\\Facultate\\CSSO\\Laboratoare", NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		HumanReadableErrorExit();
		return 1;
	}

	if (!CreateDirectory("C:\\Facultate\\CSSO\\Laboratoare\\Week1", NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		HumanReadableErrorExit();
		return 1;
	}

	if (!CreateDirectory("C:\\Facultate\\CSSO\\Laboratoare\\Week1\\Rezultate", NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		HumanReadableErrorExit();
		return 1;
	}

	HANDLE sumarFileHandle = CreateFile(
		"C:\\Facultate\\CSSO\\Laboratoare\\Week1\\Rezultate\\sumar.txt",
		FILE_APPEND_DATA,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (sumarFileHandle == INVALID_HANDLE_VALUE)
	{
		HumanReadableErrorExit();
		return 1;
	}

	CloseHandle(sumarFileHandle);

	printf("Fisierul 'sumar.txt' a fost creat!\n");

	char word[100];
	char directory[200];

	printf("Cuvant: ");
	scanf_s("%99s", word, sizeof(word));

	printf("Director: ");
	scanf_s("%199s", directory, sizeof(directory));

	SearchFiles(directory, word);

	return 0;
}

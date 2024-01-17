#include <iostream>
#include <stdio.h>
#include <Windows.h>

#define WORK_BUFFER_SIZE 0x4000

void printData(const char* fieldHeaderName, const char* fieldName, BYTE* buffer, DWORD offset, DWORD size)
{
    DWORD value;
    if (size == 2)
    {
        value = ((WORD*)(buffer + offset))[0];
    }
    else
    {
        value = ((DWORD*)(buffer + offset))[0];
    }
    printf("[%s][%s] %d\n", fieldHeaderName, fieldName, value);
}

void printHeaderData(BYTE* dataHeader)
{
    printData("Bitmap file header", "Header", dataHeader, 0, 2);
    printData("Bitmap file header", "File Size", dataHeader, 2, 4);
    printData("Bitmap file header", "Reserved", dataHeader, 6, 2);
    printData("Bitmap file header", "Reserved", dataHeader, 8, 2);
    printData("Bitmap file header", "The Offset", dataHeader, 10, 4);
    printData("DIB Header", "DIB Header Size", dataHeader, 14, 4);
    printData("DIB Header", "Bitmap Width", dataHeader, 18, 4);
    printData("DIB Header", "Bitmap Height", dataHeader, 22, 4);
    printData("DIB Header", "Number of color planes", dataHeader, 26, 2);
    printData("DIB Header", "Number of bits per pixel", dataHeader, 28, 2);
    printData("DIB Header", "Compression Method", dataHeader, 30, 4);
    printData("DIB Header", "Image Size", dataHeader, 34, 4);
    printData("DIB Header", "Horizontal Resolution", dataHeader, 38, 4);
    printData("DIB Header", "Vertical Resolution", dataHeader, 42, 4);
    printData("DIB Header", "Number of colors in the color palette", dataHeader, 46, 4);
    printData("DIB Header", "Number of important colors used", dataHeader, 50, 4);
}

void processImage(const std::wstring& inputPath, const std::wstring& outputPathGray, const std::wstring& outputPathInv)
{
    HANDLE fileHandler = CreateFile(inputPath.c_str(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandler == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Error at opening file %s; Error code: %d\n", inputPath.c_str(), GetLastError());
        return;
    }

    BYTE dataHeader[0x36];
    DWORD bytesRead;
    BYTE workBuffer[WORK_BUFFER_SIZE];

    if (!ReadFile(fileHandler, dataHeader, 0x36, &bytesRead, NULL))
    {
        wprintf(L"Error at reading file %s; Error code: %d\n", inputPath.c_str(), GetLastError());
        CloseHandle(fileHandler);
        return;
    }

    printHeaderData(dataHeader);

    HANDLE outputHandlerGray = CreateFile(outputPathGray.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE outputHandlerInv = CreateFile(outputPathInv.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    BYTE modifiedDataHeader[0x36];
    memcpy(modifiedDataHeader, dataHeader, 0x36);

    WriteFile(outputHandlerGray, modifiedDataHeader, 0x36, NULL, NULL);
    WriteFile(outputHandlerInv, modifiedDataHeader, 0x36, NULL, NULL);

    BYTE resultBufferGray[WORK_BUFFER_SIZE];
    BYTE resultBufferInv[WORK_BUFFER_SIZE];
    memset(resultBufferGray, 0, WORK_BUFFER_SIZE);
    memset(resultBufferInv, 0, WORK_BUFFER_SIZE);
    DWORD readBytes = 0;

    LARGE_INTEGER frequency, startGray, endGray, startInv, endInv;

    QueryPerformanceFrequency(&frequency);

    // Reset timer before the loop starts
    QueryPerformanceCounter(&startGray);

    while (ReadFile(fileHandler, workBuffer, WORK_BUFFER_SIZE, &readBytes, NULL) && readBytes != 0)
    {
        for (int counter = 0; counter < readBytes; counter += 4)
        {
            // grayscale
            resultBufferGray[counter] = 0.299 * workBuffer[counter] + 0.587 * workBuffer[counter + 1] + 0.114 * workBuffer[counter + 2];
            resultBufferGray[counter + 1] = 0.299 * workBuffer[counter] + 0.587 * workBuffer[counter + 1] + 0.114 * workBuffer[counter + 2];
            resultBufferGray[counter + 2] = 0.299 * workBuffer[counter] + 0.587 * workBuffer[counter + 1] + 0.114 * workBuffer[counter + 2];
        }

        WriteFile(outputHandlerGray, resultBufferGray, readBytes, NULL, NULL);
    }

    QueryPerformanceCounter(&endGray);

    // Calculate and print execution time for grayscale operation
    double elapsedSecondsGray = (double)(endGray.QuadPart - startGray.QuadPart) / frequency.QuadPart;
    wprintf(L"Grayscale operation on %s: %f seconds\n", inputPath.c_str(), elapsedSecondsGray);

    // Reset timer for inverse color operation
    QueryPerformanceCounter(&startInv);

    // Reset result buffer for inverse color operation
    memset(resultBufferInv, 0, WORK_BUFFER_SIZE);

    // Reset file pointer to the beginning for inverse color operation
    SetFilePointer(fileHandler, 0x36, NULL, FILE_BEGIN);

    while (ReadFile(fileHandler, workBuffer, WORK_BUFFER_SIZE, &readBytes, NULL) && readBytes != 0)
    {
        for (int counter = 0; counter < readBytes; counter += 4)
        {
            // inverse colors
            resultBufferInv[counter] = 0xFF - workBuffer[counter];
            resultBufferInv[counter + 1] = 0xFF - workBuffer[counter + 1];
            resultBufferInv[counter + 2] = 0xFF - workBuffer[counter + 2];
        }

        WriteFile(outputHandlerInv, resultBufferInv, readBytes, NULL, NULL);
    }

    QueryPerformanceCounter(&endInv);

    // Calculate and print execution time for inverse color operation
    double elapsedSecondsInv = (double)(endInv.QuadPart - startInv.QuadPart) / frequency.QuadPart;
    wprintf(L"Inverse color operation on %s: %f seconds\n", inputPath.c_str(), elapsedSecondsInv);

    CloseHandle(outputHandlerGray);
    CloseHandle(outputHandlerInv);
    CloseHandle(fileHandler);
}


int main()
{
    std::wstring inputDir = L"C:\\Facultate\\CSSO\\Week6\\date\\";
    std::wstring outputDirGray = L"C:\\Facultate\\CSSO\\Week6\\rezultate\\Grayscale\\";
    std::wstring outputDirInv = L"C:\\Facultate\\CSSO\\Week6\\rezultate\\Inversare\\";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((inputDir + L"*.bmp").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Error finding files in directory; Error code: %d\n", GetLastError());
        return 0;
    }

    do
    {

        LARGE_INTEGER frequency, start, end;

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);


        std::wstring inputFile = inputDir + findFileData.cFileName;
        std::wstring outputFileGray = outputDirGray + findFileData.cFileName;
        std::wstring outputFileInv = outputDirInv + findFileData.cFileName;

        processImage(inputFile, outputFileGray, outputFileInv);

        QueryPerformanceCounter(&end);

        // Calculate and print total execution time for the file
        double elapsedSeconds = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
        wprintf(L"Total processing time for %s: %f seconds\n", findFileData.cFileName, elapsedSeconds);

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    return 0;
}
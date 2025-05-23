#include "pch.h"

#include "utils.h"
#include <memory>
#include <string>
#include <strsafe.h>
#include <tchar.h>
#include <windows.h>

#pragma comment(lib, "Version.lib")

CString GetComputerNameMFC() {
    TCHAR nameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(nameBuffer) / sizeof(nameBuffer[0]);

    if (GetComputerName(nameBuffer, &size)) {
        return CString(nameBuffer);
    } else {
        return _T("Unknown");
    }
}

CString GetUsernameMFC() {
    TCHAR userName[UNLEN + 1];
    DWORD size = UNLEN + 1;

    if (GetUserName(userName, &size)) {
        return CString(userName);
    } else {
        return _T("UnknownUser");
    }
}

CString ReadIniValue(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath) {
    TCHAR buffer[256];
    GetPrivateProfileString(section, key, defaultValue, buffer, sizeof(buffer) / sizeof(TCHAR),
                            filePath);
    return CString(buffer);
}

CString GetIniFilePath(LPCTSTR iniFileName) {
    CString path = GetModulePath();
    if (path.IsEmpty()) {
        return _T(""); // Failure case
    }
    int pos = path.ReverseFind(_T('\\'));
    if (pos != -1) {
        path = path.Left(pos + 1); // Keep the directory path
    }

    path += iniFileName; // Append the ini filename
    return path;
}

CString GetIsoTimestamp() {
    SYSTEMTIME stLocal;
    GetLocalTime(&stLocal); // Local time for correct timezone offset

    // Simulate nanoseconds (7-digit fractional second)
    int microFraction =
        stLocal.wMilliseconds * 10000 + rand() % 10000; // Rough simulation of 7 digits

    // Get timezone offset
    TIME_ZONE_INFORMATION tzi;
    GetTimeZoneInformation(&tzi);

    int biasMinutes = -tzi.Bias;
    int hoursOffset = biasMinutes / 60;
    int minutesOffset = abs(biasMinutes % 60);

    CString iso;
    iso.Format(_T("%04d-%02d-%02dT%02d:%02d:%02d.%07d%+03d:%02d"), stLocal.wYear, stLocal.wMonth,
               stLocal.wDay, stLocal.wHour, stLocal.wMinute, stLocal.wSecond, microFraction,
               hoursOffset, minutesOffset);

    return iso;
}

std::string GetLastErrorString(DWORD errorCode = GetLastError()) {
    HMODULE hModule = LoadLibraryEx(_T("wininet.dll"), nullptr, LOAD_LIBRARY_AS_DATAFILE);
    if (!hModule)
        return "Failed to load wininet.dll";

    LPWSTR lpMsgBuf = nullptr;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE |
                      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  hModule, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf,
                  0, nullptr);

    std::string result;
    if (lpMsgBuf) {
        CW2A ansiStr(lpMsgBuf); // Convert wide string to ANSI using ATL
        result = ansiStr;
        LocalFree(lpMsgBuf);
    } else {
        result = "Unknown error";
    }

    return result;
}

CString GetModulePath() {
    CString path;
    DWORD bufferSize = MAX_PATH;

    while (true) {
        DWORD len = ::GetModuleFileName(NULL, path.GetBufferSetLength(bufferSize), bufferSize);
        if (len == 0) {
            path.ReleaseBuffer();
            return _T(""); // Failure case
        }

        if (len < bufferSize - 1) {
            path.ReleaseBuffer(len);
            break; // Success
        }

        // Buffer may have been too small, increase and retry
        bufferSize *= 2;
    }

    return path;
}

CString GetAppVersion() {
    CString versionStr = _T("Unknown");

    // Get the full path of the executable
    CString szFilePath = GetModulePath();

    DWORD dummy;
    DWORD verSize = GetFileVersionInfoSize((LPCTSTR)szFilePath, &dummy);
    if (verSize == 0)
        return versionStr;

    std::unique_ptr<BYTE[]> verData(new BYTE[verSize]);

    if (!GetFileVersionInfo((LPCTSTR)szFilePath, 0, verSize, verData.get()))
        return versionStr;

    VS_FIXEDFILEINFO* pFileInfo = nullptr;
    UINT len = 0;

    if (VerQueryValue(verData.get(), _T("\\"), reinterpret_cast<LPVOID*>(&pFileInfo), &len)) {
        if (pFileInfo->dwSignature == VS_FFI_SIGNATURE) {
            versionStr.Format(_T("%u.%u.%u"), LOWORD(pFileInfo->dwFileVersionMS),
                              HIWORD(pFileInfo->dwFileVersionLS),
                              LOWORD(pFileInfo->dwFileVersionLS));
        }
    }

    return versionStr;
}
#pragma once
#include <string>

CString GetComputerNameMFC();
CString GetUsernameMFC();
CString ReadIniValue(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath);
CString GetIsoTimestamp();
CString GetIniFilePath(LPCTSTR iniFileName);
std::string GetLastErrorString(DWORD errorCode);
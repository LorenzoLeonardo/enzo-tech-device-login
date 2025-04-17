#pragma once

CString GetComputerNameMFC();
CString GetUsernameMFC();
CString ReadIniValue(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath);
void WriteIniValue(LPCTSTR section, LPCTSTR key, LPCTSTR value, LPCTSTR filePath);
CString GetIsoTimestamp();
CString GetIniFilePath(LPCTSTR iniFileName);

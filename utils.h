#pragma once

CString GetComputerNameMFC();
CString GetUsernameMFC();
CString ReadIniValue(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath);
CString GetIsoTimestamp();
CString GetIniFilePath(LPCTSTR iniFileName);
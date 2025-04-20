#pragma once
#include <string>

CString GetComputerNameMFC();
CString GetUsernameMFC();
CString ReadIniValue(LPCTSTR section, LPCTSTR key, LPCTSTR defaultValue, LPCTSTR filePath);
CString GetIsoTimestamp();
CString GetIniFilePath(LPCTSTR iniFileName);
std::string GetLastErrorString(DWORD errorCode);

#define LAMBDA_SHOW_MSGBOX_OK(dlg)                                                                 \
    [dlg](const CString& title, const CString& msg) {                                              \
        ::MessageBox((dlg)->GetSafeHwnd(), msg, title, MB_OK | MB_ICONINFORMATION);                \
    }
#define LAMBDA_SHOW_MSGBOX_ERROR(dlg)                                                              \
    [dlg](const CString& title, const CString& msg) {                                              \
        ::MessageBox((dlg)->GetSafeHwnd(), msg, title, MB_OK | MB_ICONERROR);                      \
    }

static inline CString LoadLocalizedString(UINT id) {
    CString strItem;
    VERIFY(strItem.LoadString(id));

    return std::move(strItem);
}
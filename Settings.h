#pragma once
#include "utils.h"
#include <Windows.h> // For GetPrivateProfileString
#include <afx.h>     // For CString
#include <memory>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

class Settings {
  private:
    CString m_endPoint;
    CString m_hostName;

    // Private constructor reads the settings.ini file
    Settings() {
        CString path = GetIniFilePath(_T("settings.ini"));

        TCHAR buffer[MAX_PATH] = {0};
        ::GetPrivateProfileString(_T("Network"), _T("Endpoint"), _T("http://127.0.0.1:3443"),
                                  buffer, MAX_PATH, path);
        m_endPoint = buffer;

        // Ensure the string is null-terminated
        if (m_endPoint.GetLength() == 0) {
            m_endPoint = _T("http://127.0.0.1:3443");
        }

        URL_COMPONENTS urlComp;
        // Use m_endPoint length to allocate enough space for the hostname
        DWORD dwSize = m_endPoint.GetLength() + 1; // +1 for null-terminator

        // Allocate memory for the hostname using unique_ptr
        std::unique_ptr<TCHAR[]> szHostName(new TCHAR[dwSize]); // Unique pointer

        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.lpszHostName = szHostName.get(); // Use get() to access raw pointer
        urlComp.dwHostNameLength = dwSize;

        if (InternetCrackUrl(m_endPoint, 0, 0, &urlComp)) {
            CString hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);

            // If localhost or 127.0.0.1, include the port
            if (hostName.CompareNoCase(_T("localhost")) == 0 ||
                hostName.CompareNoCase(_T("127.0.0.1")) == 0) {
                m_hostName.Format(_T("%s:%d"), (LPCTSTR)hostName, urlComp.nPort);
            } else {
                m_hostName = hostName;
            }
        }
    }

    // Delete copy constructor and assignment operator
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

  public:
    static Settings& GetInstance() {
        static Settings instance;
        return instance;
    }

    CString Url() const { return m_endPoint; }
    CString HostName() const { return m_hostName; }
};
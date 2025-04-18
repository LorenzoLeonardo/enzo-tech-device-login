#pragma once
#include <Windows.h> // For GetPrivateProfileString
#include <afx.h>     // For CString
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

class Settings {
  private:
    CString m_endPoint;
    CString m_hostName;

    // Private constructor reads the settings.ini file
    Settings() {
        CString path;
        ::GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
        path.ReleaseBuffer();
        path = path.Left(path.ReverseFind(_T('\\')) + 1) + _T("settings.ini");

        TCHAR buffer[MAX_PATH] = {0};
        ::GetPrivateProfileString(_T("Network"), _T("Endpoint"), _T("http://127.0.0.1:3443"),
                                  buffer, MAX_PATH, path);
        m_endPoint = buffer;

        URL_COMPONENTS urlComp;
        TCHAR szHostName[256] = {0};

        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.lpszHostName = szHostName;
        urlComp.dwHostNameLength = _countof(szHostName);

        if (InternetCrackUrl(m_endPoint, 0, 0, &urlComp)) {
            CString hostName(szHostName, urlComp.dwHostNameLength);

            // If localhost or 127.0.0.1, include the port
            if (hostName.CompareNoCase(_T("localhost")) == 0 ||
                hostName.CompareNoCase(_T("127.0.0.1")) == 0) {
                m_hostName.Format(_T("%s:%d"), hostName.GetString(), urlComp.nPort);
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
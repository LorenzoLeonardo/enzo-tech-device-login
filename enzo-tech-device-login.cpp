
// enzo-tech-device-login.cpp : Defines the class behaviors for the application.
//
#include "pch.h"

#include "CLoginDialog.h"
#include "CTaskProgressDlg.h"
#include "Communicator.h"
#include "Settings.h"
#include "Uuid.h"
#include "enzo-tech-device-login.h"
#include "enzo-tech-device-loginDlg.h"
#include "framework.h"
#include "utils.h"
#include <AsyncMFCDialog/AsyncGenericDialog.hpp>
#include <atlconv.h>
#include <thread>

#ifdef _DEBUG
#    define new DEBUG_NEW
#endif

// CenzotechdeviceloginApp

BEGIN_MESSAGE_MAP(CenzotechdeviceloginApp, CWinApp)
ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

static bool IsDefaultSession(const CString& session_id, const CString& user_id) {
    return (session_id == _T("default_session_id") || user_id == _T("default_user_id"));
}

static bool
PerformLoginFlow(const CString& path, CTaskProgressDlg* pWaitDlg, Provider provider,
                 std::function<void(const CString&, const CString&)> showMessageCallback) {
    std::string uuid_s = generate_uuid();
    CString uuid(CA2T(uuid_s.c_str(), CP_UTF8));
    CString url;
    url.Format(_T("%s/auth?login=%s&session_id=%s"), Settings::GetInstance().Url().GetString(),
               Providers::ToString(provider).GetString(), uuid.GetString());
    ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOWNORMAL);

    int attempts = 0;
    while (!pWaitDlg->HasCancelled()) {
        ApiResponse resp =
            HttpPost<PollRequest>(PollRequest{uuid_s}, _T("enzotechcomputersolutions.com"),
                                  _T("/applications/poll_login"));
        if (std::holds_alternative<PollResponse>(resp)) {
            PollResponse response = std::get<PollResponse>(resp);
            CString user(CA2T(response.user_id.c_str(), CP_UTF8));
            CString name(CA2T(response.name.c_str(), CP_UTF8));
            CString email(CA2T(response.email.c_str(), CP_UTF8));
            CString login_status(CA2T(response.login_status.c_str(), CP_UTF8));
            BOOL success = WritePrivateProfileString(_T("User"), _T("user_id"), user, path) &&
                           WritePrivateProfileString(_T("User"), _T("session_id"), uuid, path) &&
                           WritePrivateProfileString(_T("User"), _T("name"), name, path) &&
                           WritePrivateProfileString(_T("User"), _T("email"), email, path) &&
                           WritePrivateProfileString(_T("User"), _T("action"), login_status, path);
            if (!success) {
                showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                    LoadLocalizedString(IDS_ERROR_INI_FILE_WRITE));
                return false;
            }
            return true;
        } else if (std::holds_alternative<PollResponseError>(resp)) {
            PollResponseError response = std::get<PollResponseError>(resp);
            if (response.error != ErrorCodes::authorization_pending) {
                CString error;
                error.Format(_T("Server error: %d"), response.error);
                showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION), error.GetString());
                return false;
            }
        } else if (std::holds_alternative<HttpError>(resp)) {
            HttpError response = std::get<HttpError>(resp);
            CString error(response.http_error.c_str());

            showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION), error.GetString());
            return false;
        } else {
            showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                LoadLocalizedString(IDS_ERROR_UNKNOWN));
            return false;
        }
        Sleep(5000);
    }
    return false;
}

static bool
CheckExistingSession(const CString& session_id, const CString& path,
                     std::function<void(const CString&, const CString&)> showMessageCallback) {
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
    CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
    CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
    CString device_id = GetComputerNameMFC();
    CString username = GetUsernameMFC();
    CString timestamp = GetIsoTimestamp();
    CString action = _T("checksession");

    ApiResponse resp = HttpPost<DeviceEvent>(
        DeviceEvent{
            std::string(CW2A(session_id.GetString(), CP_UTF8)),
            std::string(CW2A(user_id.GetString(), CP_UTF8)),
            std::string(CW2A(username.GetString(), CP_UTF8)),
            std::string(CW2A(timestamp.GetString(), CP_UTF8)),
            std::string(CW2A(action.GetString(), CP_UTF8)),
            std::string(CW2A(device_id.GetString(), CP_UTF8)),
        },
        Settings::GetInstance().HostName(), _T("/applications/device_login"));

    if (std::holds_alternative<DeviceLoginResponseSuccess>(resp)) {
        DeviceLoginResponseSuccess response = std::get<DeviceLoginResponseSuccess>(resp);
        CString login_status(CA2T(response.login_status.c_str(), CP_UTF8));
        BOOL success = WritePrivateProfileString(_T("User"), _T("action"), login_status, path);
        if (!success) {
            showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                LoadLocalizedString(IDS_ERROR_INI_FILE_WRITE));
            return false;
        }
        return true;
    } else if (std::holds_alternative<DeviceLoginResponseError>(resp)) {
        DeviceLoginResponseError response = std::get<DeviceLoginResponseError>(resp);
        if (response.error_code == ErrorCodes::invalid_grant) {
            BOOL success =
                WritePrivateProfileString(_T("User"), _T("user_id"), _T("default_user_id"), path) &&
                WritePrivateProfileString(_T("User"), _T("session_id"), _T("default_session_id"),
                                          path);
            if (!success) {
                showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                    LoadLocalizedString(IDS_ERROR_INI_FILE_WRITE));
                return false;
            }
            showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                LoadLocalizedString(IDS_ERROR_SESSION_EXPIRED));
        } else {
            showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                LoadLocalizedString(IDS_ERROR_SERVER));
        }
    } else if (std::holds_alternative<HttpError>(resp)) {
        HttpError response = std::get<HttpError>(resp);

        CString error(response.http_error.c_str());
        showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION), error.GetString());
    } else {
        showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                            LoadLocalizedString(IDS_ERROR_UNKNOWN));
    }

    return false;
}

static bool
GetServerVersion(const CString& path,
                 std::function<void(const CString&, const CString&)> showMessageCallback) {
    ApiResponse resp =
        HttpGet<CString>(_T(""), Settings::GetInstance().HostName(), _T("/server_info"));

    if (std::holds_alternative<PackageName>(resp)) {
        PackageName response = std::get<PackageName>(resp);
        CString Name(CA2T(response.Name.c_str(), CP_UTF8));
        CString Version(CA2T(response.Version.c_str(), CP_UTF8));
        BOOL success = WritePrivateProfileString(_T("User"), _T("Servername"), Name, path) &&
                       WritePrivateProfileString(_T("User"), _T("Serverversion"), Version, path);
        if (!success) {
            showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                                LoadLocalizedString(IDS_ERROR_INI_FILE_WRITE));
            return false;
        }
        return true;
    } else if (std::holds_alternative<HttpError>(resp)) {
        HttpError response = std::get<HttpError>(resp);
        CString error(response.http_error.c_str());

        showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION), error.GetString());
    } else {
        showMessageCallback(LoadLocalizedString(IDS_TITLE_INFORMATION),
                            LoadLocalizedString(IDS_ERROR_UNKNOWN));
    }

    return false;
}

// CenzotechdeviceloginApp construction
CenzotechdeviceloginApp::CenzotechdeviceloginApp() {
    // support Restart Manager
    m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

void CenzotechdeviceloginApp::ShowMainDialog() {
    CenzotechdeviceloginDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK) {
        // Handle OK
    } else if (nResponse == IDCANCEL) {
        // Handle Cancel
    } else if (nResponse == -1) {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed.\n");
    }
}

// The one and only CenzotechdeviceloginApp object
CenzotechdeviceloginApp theApp;

// CenzotechdeviceloginApp initialization
BOOL CenzotechdeviceloginApp::InitInstance() {
    TRACE(traceAppMsg, 0, "Enzo Tech Timekeeping Badger - Started.\n");

    HANDLE hMutex = CreateMutex(NULL, FALSE, AfxGetApp()->m_pszAppName);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        ::MessageBox(nullptr, LoadLocalizedString(IDS_ERROR_ANOTHER_INSTANCE),
                     LoadLocalizedString(IDS_TITLE_INFORMATION), MB_OK | MB_ICONERROR);
        if (hMutex) {
            CloseHandle(hMutex);
        }
        return FALSE; // Exit the application
    }

    CString path = GetIniFilePath(_T("user.ini"));
    CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
    bool isDefault = IsDefaultSession(session_id, user_id);

    CLoginDialog loginDlg;
    if (isDefault) {
        INT_PTR nResponse = loginDlg.DoModal();
        if (nResponse == IDCANCEL) {
            if (hMutex) {
                CloseHandle(hMutex);
            }
            return FALSE;
        }
    }
    Provider provider = loginDlg.Provider();

    INITCOMMONCONTROLSEX InitCtrls = {sizeof(InitCtrls), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&InitCtrls);
    CWinApp::InitInstance();
    AfxEnableControlContainer();

    std::unique_ptr<CShellManager> pShellManager(new CShellManager);
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    auto pAuthDlg = std::make_shared<CTaskProgressDlg>();
    pAuthDlg->Create(IDD_AUTH_PROGRESS, AfxGetMainWnd());
    pAuthDlg->SetWindowText(_T("Connecting to ") + Settings::GetInstance().Url());

    bool is_success =
        TAsyncGenericDialog<CTaskProgressDlg, bool>(pAuthDlg, [&](CTaskProgressDlg* dlg) {
            return (isDefault
                        ? PerformLoginFlow(path, dlg, provider, LAMBDA_SHOW_MSGBOX_ERROR(dlg))
                        : CheckExistingSession(session_id, path, LAMBDA_SHOW_MSGBOX_ERROR(dlg))) &&
                   GetServerVersion(path, LAMBDA_SHOW_MSGBOX_ERROR(dlg));
        }).Await();

    if (::IsWindow(pAuthDlg->GetSafeHwnd())) {
        pAuthDlg->DestroyWindow();
    }

    if (is_success) {
        ShowMainDialog();
#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
        ControlBarCleanUp();
#endif

        if (hMutex) {
            CloseHandle(hMutex);
        }
        return TRUE;
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    if (hMutex) {
        CloseHandle(hMutex);
    }
    return FALSE;
}

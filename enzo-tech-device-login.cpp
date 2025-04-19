
// enzo-tech-device-login.cpp : Defines the class behaviors for the application.
//
#include "pch.h"

#include "CAuthProgressDlg.h"
#include "Communicator.h"
#include "MessageBoxCustomizer.h"
#include "Settings.h"
#include "Uuid.h"
#include "enzo-tech-device-login.h"
#include "enzo-tech-device-loginDlg.h"
#include "framework.h"
#include "utils.h"
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

static bool PerformLoginFlow(const CString& path, CAuthProgressDlg* pWaitDlg) {
    std::string uuid_s = generate_uuid();
    CString uuid(CA2T(uuid_s.c_str(), CP_UTF8));
    CString url;
    url.Format(_T("%s/auth?login=Google&session_id=%s"), Settings::GetInstance().Url().GetString(),
               uuid.GetString());
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
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Error writing to ini file."),
                             _T("Information"), MB_OK | MB_ICONERROR);
                return false;
            }
            return true;
        } else if (std::holds_alternative<PollResponseError>(resp)) {
            PollResponseError response = std::get<PollResponseError>(resp);
            if (response.error != ErrorCodes::authorization_pending) {
                CString error;
                error.Format(_T("Server error: %d"), response.error);
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), error.GetString(), _T("Information"),
                             MB_OK | MB_ICONERROR);
                return false;
            }
        } else if (std::holds_alternative<HttpError>(resp)) {
            HttpError response = std::get<HttpError>(resp);

            CString error(response.http_error.c_str());
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), error.GetString(), _T("Information"),
                         MB_OK | MB_ICONERROR);
            return false;
        } else {
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Unknown error"), _T("Information"),
                         MB_OK | MB_ICONERROR);
            return false;
        }
        Sleep(5000);
    }
    return false;
}

static bool CheckExistingSession(const CString& session_id, const CString& path) {
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
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Error writing to ini file."),
                         _T("Information"), MB_OK | MB_ICONERROR);
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
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Error writing to ini file."),
                             _T("Information"), MB_OK | MB_ICONERROR);
                return false;
            }
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
                         _T("Session has expired. Please run the program again."),
                         _T("Information"), MB_OK | MB_ICONERROR);
        } else {
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Server Error. Please try again."),
                         _T("Information"), MB_OK | MB_ICONERROR);
        }
    } else if (std::holds_alternative<HttpError>(resp)) {
        HttpError response = std::get<HttpError>(resp);

        CString error(response.http_error.c_str());
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), error.GetString(), _T("Information"),
                     MB_OK | MB_ICONERROR);
    } else {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Unknown error"), _T("Information"),
                     MB_OK | MB_ICONERROR);
    }

    return false;
}

static bool GetServerVersion(const CString& path) {
    ApiResponse resp =
        HttpGet<CString>(_T(""), Settings::GetInstance().HostName(), _T("/server_info"));

    if (std::holds_alternative<PackageName>(resp)) {
        PackageName response = std::get<PackageName>(resp);
        CString Name(CA2T(response.Name.c_str(), CP_UTF8));
        CString Version(CA2T(response.Version.c_str(), CP_UTF8));
        BOOL success = WritePrivateProfileString(_T("User"), _T("Servername"), Name, path) &&
                       WritePrivateProfileString(_T("User"), _T("Serverversion"), Version, path);
        if (!success) {
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Error writing to ini file."),
                         _T("Information"), MB_OK | MB_ICONERROR);
            return false;
        }
        return true;
    } else if (std::holds_alternative<HttpError>(resp)) {
        HttpError response = std::get<HttpError>(resp);

        CString error(response.http_error.c_str());
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), error.GetString(), _T("Information"),
                     MB_OK | MB_ICONERROR);
    } else {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Unknown error"), _T("Information"),
                     MB_OK | MB_ICONERROR);
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
    MessageBoxCustomizer::Instance().Initialize();

    HANDLE hMutex = CreateMutex(NULL, FALSE, AfxGetApp()->m_pszAppName);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
                     _T("Another instance of the application is already running."),
                     _T("Information"), MB_OK | MB_ICONERROR);
        return FALSE; // Exit the application
    }
    INITCOMMONCONTROLSEX InitCtrls = {sizeof(InitCtrls), ICC_WIN95_CLASSES};
    InitCommonControlsEx(&InitCtrls);
    CWinApp::InitInstance();
    AfxEnableControlContainer();

    std::unique_ptr<CShellManager> pShellManager(new CShellManager);
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    AfxGetApp()->m_pszAppName = _tcsdup(_T("Notification"));

    CString path = GetIniFilePath(_T("user.ini"));
    CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
    bool isDefault = IsDefaultSession(session_id, user_id);

    if (IsDefaultSession(session_id, user_id)) {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
                     _T("Session ID or User ID is not set. Please log in first."),
                     _T("Information"), MB_OK | MB_ICONINFORMATION);
    }

    auto pWaitDlg = std::make_unique<CAuthProgressDlg>();

    pWaitDlg->Create(IDD_AUTH_PROGRESS, AfxGetMainWnd());
    pWaitDlg->ShowWindow(SW_SHOW);

    std::atomic<bool> isDone{false};
    bool is_success = false;

    std::thread authThread([&]() {
        is_success = (isDefault ? PerformLoginFlow(path, pWaitDlg.get())
                                : CheckExistingSession(session_id, path)) &&
                     GetServerVersion(path);
        isDone = true;
    });
    authThread.detach();

    // Pump messages while waiting
    MSG msg;
    while (!isDone) {
        while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        Sleep(10); // Let CPU rest
    }

    // Done — close the dialog
    if (pWaitDlg && ::IsWindow(pWaitDlg->GetSafeHwnd())) {
        pWaitDlg->DestroyWindow();
    }

    if (is_success) {
        ShowMainDialog();
    }

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    return FALSE;
}

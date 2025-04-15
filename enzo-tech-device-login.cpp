
// enzo-tech-device-login.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "enzo-tech-device-login.h"
#include "enzo-tech-device-loginDlg.h"
#include "Communicator.h"
#include "Uuid.h"
#include "utils.h"

#include <atlconv.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CenzotechdeviceloginApp

BEGIN_MESSAGE_MAP(CenzotechdeviceloginApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

static bool IsDefaultSession(const CString& session_id, const CString& user_id) {
    return (session_id == _T("default_session_id") || user_id == _T("default_user_id"));
}

static bool PerformLoginFlow(const CString& path) {
    std::string uuid_s = generate_uuid();
    CString uuid(CA2T(uuid_s.c_str(), CP_UTF8));
    CString url;
    url.Format(_T("https://enzotechcomputersolutions.com/auth?login=Google&session_id=%s"), (LPCTSTR)uuid);
    ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOWNORMAL);

    int attempts = 0;
    while (attempts++ < 12) {
        ApiResponse resp = HttpPost<PollRequest>(PollRequest{ uuid_s }, _T("enzotechcomputersolutions.com"), _T("/poll_login"));
        if (std::holds_alternative<PollResponse>(resp)) {
            PollResponse response = std::get<PollResponse>(resp);
            CString user(CA2T(response.user_id.c_str(), CP_UTF8));
            CString name(CA2T(response.name.c_str(), CP_UTF8));
            CString email(CA2T(response.email.c_str(), CP_UTF8));
            WriteIniValue(_T("User"), _T("user_id"), user, path);
            WriteIniValue(_T("User"), _T("session_id"), uuid, path);
            WriteIniValue(_T("User"), _T("name"), name, path);
            WriteIniValue(_T("User"), _T("email"), email, path);
            CString welcome;
			welcome.Format(_T("Welcome %s (%s)"), (LPCTSTR)name, (LPCTSTR)email);
            AfxMessageBox(welcome, MB_OK | MB_ICONINFORMATION);
            return true;
        }
        Sleep(5000);
    }

    AfxMessageBox(_T("Login timeout. Please try again."), MB_OK | MB_ICONERROR);
    return false;
}

static bool CheckExistingSession(const CString& session_id, const CString& path) {
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
    CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
    CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
    CString device_id = GetComputerNameMFC();
    CString username = GetUsernameMFC();
    CString timestamp = GetIsoTimestamp();
    CString action = ReadIniValue(_T("User"), _T("action"), _T("logout"), path);

    ApiResponse resp = HttpPost<DeviceEvent>(DeviceEvent
        {
            std::string(CW2A(session_id.GetString(), CP_UTF8)),
            std::string(CW2A(user_id.GetString(), CP_UTF8)),
            std::string(CW2A(username.GetString(), CP_UTF8)),
            std::string(CW2A(timestamp.GetString(), CP_UTF8)),
            std::string(CW2A(action.GetString(), CP_UTF8)),
            std::string(CW2A(device_id.GetString(), CP_UTF8)),
        }, _T("enzotechcomputersolutions.com"), _T("/device_login"));

    if (std::holds_alternative<DeviceLoginResponseSuccess>(resp)) {

        CString welcome;
        welcome.Format(_T("Welcome back %s (%s)"), (LPCTSTR)name, (LPCTSTR)email);
        AfxMessageBox(welcome, MB_OK | MB_ICONINFORMATION);
        return true;
    }
    else if (std::holds_alternative<DeviceLoginResponseError>(resp)) {
        DeviceLoginResponseError response = std::get<DeviceLoginResponseError>(resp);
        if (response.error_code == ErrorCodes::invalid_grant) {
            WriteIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
            WriteIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
            AfxMessageBox(_T("Session has expired. Please run the program again."), MB_OK | MB_ICONERROR);
        }
        else {
            AfxMessageBox(_T("Server Error. Please try again."), MB_OK | MB_ICONERROR);
        }
    }

    return false;
}

// CenzotechdeviceloginApp construction
CenzotechdeviceloginApp::CenzotechdeviceloginApp()
{
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
    }
    else if (nResponse == IDCANCEL) {
        // Handle Cancel
    }
    else if (nResponse == -1) {
        TRACE(traceAppMsg, 0, "Warning: dialog creation failed.\n");
    }
}

// The one and only CenzotechdeviceloginApp object
CenzotechdeviceloginApp theApp;

// CenzotechdeviceloginApp initialization
BOOL CenzotechdeviceloginApp::InitInstance()
{
    INITCOMMONCONTROLSEX InitCtrls = { sizeof(InitCtrls), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&InitCtrls);
    CWinApp::InitInstance();
    AfxEnableControlContainer();

    CShellManager* pShellManager = new CShellManager;
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    CString path = GetIniFilePath(_T("user.ini"));
    CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);

    USES_CONVERSION;
    bool is_success = false;

    if (IsDefaultSession(session_id, user_id)) {
        AfxMessageBox(_T("Session ID or User ID is not set. Please log in first."));
        is_success = PerformLoginFlow(path);
    }
    else {
        is_success = CheckExistingSession(session_id, path);
    }

    if (is_success) {
        ShowMainDialog();
    }

    if (pShellManager != nullptr)
        delete pShellManager;

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
    ControlBarCleanUp();
#endif

    return FALSE;
}

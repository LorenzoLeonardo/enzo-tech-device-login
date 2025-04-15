
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


// CenzotechdeviceloginApp construction

CenzotechdeviceloginApp::CenzotechdeviceloginApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CenzotechdeviceloginApp object

CenzotechdeviceloginApp theApp;


// CenzotechdeviceloginApp initialization

BOOL CenzotechdeviceloginApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CString path = GetIniFilePath(_T("user.ini"));
	CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
	CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
	CString timestamp = GetIsoTimestamp();
	CString device_id = GetComputerNameMFC();
	bool is_success = false;

	if (session_id == _T("default_session_id") || user_id == _T("default_user_id")) {
		AfxMessageBox(_T("Session ID or User ID is not set. Please log in first."));
		CString url;
		std::string uuid_s = generate_uuid();
		CString uuid(CA2T(uuid_s.c_str(), CP_UTF8));
		url.Format(_T("https://enzotechcomputersolutions.com/auth?login=Google&session_id=%s"), (LPCTSTR)uuid);
		ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOWNORMAL);

		do {
			ApiResponse resp = HttpPost<PollRequest>(PollRequest{ uuid_s }, _T("enzotechcomputersolutions.com"), _T("/poll_login"));
			if (resp.index() == 0) { // PollResponse
				PollResponse response = std::get<PollResponse>(resp);
				CString user(CA2T(response.user_id.c_str(), CP_UTF8));
				WriteIniValue(_T("User"), _T("user_id"), user, path);
				WriteIniValue(_T("User"), _T("session_id"), uuid, path);
				AfxMessageBox(_T("Login successful!"), MB_OK | MB_ICONINFORMATION);
				is_success = true;
				break;
			}
			Sleep(5000);
		} while (true);
	} else {

	}


	/*std::string uuid = generate_uuid();
	ApiResponse resp = HttpPost<PollRequest>(PollRequest{ uuid }, _T("enzotechcomputersolutions.com"), _T("/device_login"));

	int typeIndex = resp.index();
// Use std::get to access the stored value
	if (typeIndex == 0) { // PollResponse
		PollResponse response = std::get<PollResponse>(resp);
	}
	else if (typeIndex == 1) { // PollResponseError
		PollResponseError error = std::get<PollResponseError>(resp);
	}
	else if (typeIndex == 2) { // DeviceLoginResponse
		DeviceLoginResponse loginResponse = std::get<DeviceLoginResponse>(resp);
	}*/
	if (is_success) {
		CenzotechdeviceloginDlg dlg;
		m_pMainWnd = &dlg;
		INT_PTR nResponse = dlg.DoModal();
		if (nResponse == IDOK)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with OK
		}
		else if (nResponse == IDCANCEL)
		{
			// TODO: Place code here to handle when the dialog is
			//  dismissed with Cancel
		}
		else if (nResponse == -1)
		{
			TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
			TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
		}
	}
	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}


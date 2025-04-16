
// enzo-tech-device-loginDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "enzo-tech-device-login.h"
#include "enzo-tech-device-loginDlg.h"
#include "afxdialogex.h"

#include <afxwin.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

#include "json.hpp"
using json = nlohmann::json;

#include "Communicator.h"
#include "utils.h"
#include <atlconv.h>
#include <atomic>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void SendPostJsonRequest(CString action, CString userId);
void SendGetJsonRequest(CString userId);

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CenzotechdeviceloginDlg dialog

COLORREF DLG_BACKGROUND = RGB(13, 71, 161);
COLORREF GROUP_BACKGROUND = RGB(66, 165, 245);

CenzotechdeviceloginDlg::CenzotechdeviceloginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ENZOTECHDEVICELOGIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_brBackground.CreateSolidBrush(DLG_BACKGROUND);
	m_brGroupBox.CreateSolidBrush(GROUP_BACKGROUND);
	m_customClock.SetFontStyle(_T("Yu Gothic UI"));
	m_customClock.SetFontSize(40);
	m_customClock.SetFontWeight(FW_BOLD);
	m_customClock.SetTextColor(RGB(255, 255, 255));
	m_customClock.SetTextBKColor(DLG_BACKGROUND);
	m_customClock.CreateClock();
}
CenzotechdeviceloginDlg::~CenzotechdeviceloginDlg() {
	m_customClock.DestroyClock();
}

void CenzotechdeviceloginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_LOGIN, m_ctrlBtnLogin);
	DDX_Control(pDX, IDC_BUTTON_LOGOUT, m_ctrlBtnLogout);
	DDX_Control(pDX, IDC_MY_GROUPBOX, m_myGroupBox);
	DDX_Control(pDX, IDC_STATIC_PICTURE, m_ctrlStaticLogo);
}

BEGIN_MESSAGE_MAP(CenzotechdeviceloginDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CenzotechdeviceloginDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &CenzotechdeviceloginDlg::OnBnClickedButtonLogout)
	ON_WM_CTLCOLOR()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CenzotechdeviceloginDlg::UpdateClock() {
	CClientDC cdc(this);
	m_customClock.DrawClock(&cdc, 420, 30);
}
void CenzotechdeviceloginDlg::ClockThread() {
	while (!m_bClickClose) {
		UpdateClock();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// CenzotechdeviceloginDlg message handlers

BOOL CenzotechdeviceloginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CString path = GetIniFilePath(_T("user.ini"));
	CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
	CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
	CString prev_action = ReadIniValue(_T("User"), _T("action"), _T("logout"), path);
	CString device_id = GetComputerNameMFC();
	SetDlgItemText(IDC_EDIT_NAME, name);
	SetDlgItemText(IDC_EDIT_EMAIL, email);
	SetDlgItemText(IDC_EDIT_DEVICE_ID, device_id);

	if (prev_action == _T("login")) {
		m_ctrlBtnLogin.EnableWindow(FALSE);
		m_ctrlBtnLogout.EnableWindow(TRUE);
	}
	else {
		m_ctrlBtnLogin.EnableWindow(TRUE);
		m_ctrlBtnLogout.EnableWindow(FALSE);
	}
	m_clockThread = std::thread(&CenzotechdeviceloginDlg::ClockThread, this);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CenzotechdeviceloginDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CenzotechdeviceloginDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // device context for painting

		// Call base class (important!)
		CDialogEx::OnPaint();

		// Get the group box control
		CWnd* pGroup = GetDlgItem(IDC_MY_GROUPBOX);
		if (pGroup)
		{
			CRect rect;
			pGroup->GetWindowRect(&rect);
			rect.top += 10;
			//rect.left += 10;
			ScreenToClient(&rect);

			dc.FillSolidRect(&rect, RGB(66, 165, 245));
			CDialogEx::OnPaint();
		}
	}
}

HBRUSH CenzotechdeviceloginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// Set dialog background color
	if (nCtlColor == CTLCOLOR_DLG)
	{
		return (HBRUSH)m_brBackground.GetSafeHandle();
	}

	if (pWnd->GetDlgCtrlID() == IDC_STATIC)
	{
		pDC->SetTextColor(RGB(21, 101, 192));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brBackground.GetSafeHandle();
	}

	else if (pWnd->GetDlgCtrlID() == IDC_STATIC_NAME ||
		pWnd->GetDlgCtrlID() == IDC_STATIC_EMAIL ||
		pWnd->GetDlgCtrlID() == IDC_STATIC_DEVICE ||
		pWnd->GetDlgCtrlID() == IDC_MY_GROUPBOX) {
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)m_brGroupBox.GetSafeHandle();
	}
	return hbr;
}

BOOL CenzotechdeviceloginDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	if (pWnd->GetDlgCtrlID() == IDC_BUTTON_LOGIN || pWnd->GetDlgCtrlID() == IDC_BUTTON_LOGOUT)
	{
		::SetCursor(::LoadCursor(NULL, IDC_HAND));
		return TRUE;
	}
	return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CenzotechdeviceloginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CenzotechdeviceloginDlg::OnBnClickedButtonLogin()
{
	CString path = GetIniFilePath(_T("user.ini"));
	CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
	CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
	CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
	CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
	CString device_id = GetComputerNameMFC();
	CString username = GetUsernameMFC();	
	CString timestamp = GetIsoTimestamp();	
	CString action = _T("login");	

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
		AfxMessageBox(_T("Login successful"), MB_OK | MB_ICONINFORMATION);
		WriteIniValue(_T("User"), _T("action"), _T("login"), path);
		m_ctrlBtnLogin.EnableWindow(FALSE);
		m_ctrlBtnLogout.EnableWindow(TRUE);
	} else if (std::holds_alternative<DeviceLoginResponseError>(resp)) {
		DeviceLoginResponseError response = std::get<DeviceLoginResponseError>(resp);
		if (response.error_code == ErrorCodes::invalid_grant) {
			WriteIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
			WriteIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
			AfxMessageBox(_T("Session has expired. Please run the program again."), MB_OK | MB_ICONERROR);
		}
		else {
			AfxMessageBox(_T("Server Error. Please try again."), MB_OK | MB_ICONERROR);
		}
		EndDialog(IDOK);
	}
}

void CenzotechdeviceloginDlg::OnBnClickedButtonLogout()
{
	CString path = GetIniFilePath(_T("user.ini"));
	CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
	CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
	CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
	CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
	CString device_id = GetComputerNameMFC();
	CString username = GetUsernameMFC();
	CString timestamp = GetIsoTimestamp();
	CString action = _T("logout");

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
		AfxMessageBox(_T("Logout successful"), MB_OK | MB_ICONINFORMATION);
		WriteIniValue(_T("User"), _T("action"), _T("logout"), path);
		m_ctrlBtnLogin.EnableWindow(TRUE);
		m_ctrlBtnLogout.EnableWindow(FALSE);
	} else if (std::holds_alternative<DeviceLoginResponseError>(resp)) {
		DeviceLoginResponseError response = std::get<DeviceLoginResponseError>(resp);
		if (response.error_code == ErrorCodes::invalid_grant) {
			WriteIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
			WriteIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
			AfxMessageBox(_T("Session has expired. Please run the program again."), MB_OK | MB_ICONERROR);
		}
		else {
			AfxMessageBox(_T("Server Error. Please try again."), MB_OK | MB_ICONERROR);
		}
		EndDialog(IDOK);
	}
}


void SendPostJsonRequest(CString action, CString userId)
{
	CString username = GetUsernameMFC();
	CString deviceId = GetComputerNameMFC();
	CString timestamp = GetIsoTimestamp();

	// Format JSON payload
	CString jsonData;
	jsonData.Format(_T(R"({"username":"%s","timestamp":"%s","action":"%s","user_id":"%s","device_id":"%s"})"),
		username, timestamp, action, userId, deviceId);

	// Open internet session
	HINTERNET hInternet = InternetOpen(_T("MFCApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		AfxMessageBox(_T("InternetOpen failed"));
		return;
	}

	// Connect to host
	HINTERNET hConnect = InternetConnect(hInternet, _T("enzotechcomputersolutions.com"), INTERNET_DEFAULT_HTTPS_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hConnect) {
		AfxMessageBox(_T("InternetConnect failed"));
		InternetCloseHandle(hInternet);
		return;
	}

	// Open HTTP request
	HINTERNET hRequest = HttpOpenRequest(hConnect, _T("POST"), _T("/device_login"), NULL, NULL, NULL,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
	if (!hRequest) {
		AfxMessageBox(_T("HttpOpenRequest failed"));
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return;
	}

	// Convert jsonData (CString - UTF-16) to UTF-8
	CW2A utf8Json(jsonData, CP_UTF8); // convert wide string to UTF-8
	LPCSTR utf8Body = utf8Json;
	int utf8Length = (int)strlen(utf8Body);

	// Send the request with UTF-8 body
	LPCTSTR headers = _T("Content-Type: application/json\r\n");
	BOOL success = HttpSendRequest(hRequest, headers, -1L,
		(LPVOID)utf8Body,
		utf8Length);  // size in bytes (UTF-8)

	if (success) {
		char buffer[4096] = {};
		DWORD bytesRead = 0;
		CStringA response;
		while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
			buffer[bytesRead] = '\0';
			response += buffer;
		}

		try {
			json j = json::parse((LPCSTR)response);

			// Parse the JSON response
			bool success = j.value("success", false);
			std::string error = j.value("error", "");
			std::string errorCode = j.value("error_code", "");

			// Handle the parsed values
			if (success) {
				if (userId == _T("login")) {
					AfxMessageBox(_T("Login successful"), MB_OK | MB_ICONINFORMATION);
				}
				else if (userId == _T("logout")) {
					AfxMessageBox(_T("Logout successful"), MB_OK | MB_ICONINFORMATION);

				}
			} else {
				if (errorCode == "user_not_found") {
					CString errorMessage;
					
					errorMessage.Format(_T("%s. Would you like to register?"), (LPCTSTR)CString(CA2T(error.c_str())));

					int result = AfxMessageBox(errorMessage, MB_YESNO | MB_ICONQUESTION);
					if (result == IDYES) {
						// Open registration link
						ShellExecute(NULL, _T("open"), _T("https://enzotechcomputersolutions.com/login"), NULL, NULL, SW_SHOWNORMAL);
					}
				}
				else {
					CString errorMessage;
					errorMessage.Format(_T("Operation failed. Error: %s, Code: %s"),
						(LPCTSTR)CString(CA2T(error.c_str())),
						(LPCTSTR)CString(CA2T(errorCode.c_str())));
					AfxMessageBox(errorMessage, MB_OK | MB_ICONERROR);
				}
			}
		} catch (const json::exception& e) {
			CString errorMessage;
			errorMessage.Format(_T("JSON parsing failed: %s"), (LPCTSTR)CString(CA2T(e.what())));
			AfxMessageBox(errorMessage, MB_OK | MB_ICONERROR);
		}
	}
	else {
		AfxMessageBox(_T("HttpSendRequest failed"));
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}

void SendGetJsonRequest(CString userId)
{
	HINTERNET hInternet = InternetOpen(_T("MFCApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		AfxMessageBox(_T("InternetOpen failed"));
		return;
	}

	HINTERNET hConnect = InternetConnect(hInternet, _T("enzotechcomputersolutions.com"),
		INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (!hConnect) {
		AfxMessageBox(_T("InternetConnect failed"));
		InternetCloseHandle(hInternet);
		return;
	}

	CString request;
	request.Format(_T("/device_login?user_id=%s"), (LPCTSTR)userId);

	HINTERNET hRequest = HttpOpenRequest(hConnect, _T("GET"), request,
		NULL, NULL, NULL,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);

	if (!hRequest) {
		AfxMessageBox(_T("HttpOpenRequest failed"));
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return;
	}

	LPCTSTR headers = _T("Content-Type: application/json\r\n");
	BOOL success = HttpSendRequest(hRequest, headers, -1L, NULL, 0);

	if (success) {
		CStringA response;
		char buffer[4096] = {};
		DWORD bytesRead = 0;

		while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
			buffer[bytesRead] = '\0';
			response += buffer;
		}

		try {
			json j = json::parse(response.GetString());

			bool successFlag = j.value("success", false);
			std::string error = j.value("error", "");
			std::string errorCode = j.value("error_code", "");

			if (successFlag) {
				CString message = (userId == _T("login")) ? _T("Login successful") :
					(userId == _T("logout")) ? _T("Logout successful") : _T("Success");
				AfxMessageBox(message, MB_OK | MB_ICONINFORMATION);
			}
			else {
				if (errorCode == "user_not_found") {
					CString prompt;
					prompt.Format(_T("%s. Would you like to register?"), (LPCTSTR)CString(CA2T(error.c_str())));
					int result = AfxMessageBox(prompt, MB_YESNO | MB_ICONQUESTION);
					if (result == IDYES) {
						ShellExecute(NULL, _T("open"), _T("https://enzotechcomputersolutions.com/login"), NULL, NULL, SW_SHOWNORMAL);
					}
				}
				else {
					CString errorMessage;
					errorMessage.Format(_T("Operation failed. Error: %s, Code: %s"),
						(LPCTSTR)CString(CA2T(error.c_str())),
						(LPCTSTR)CString(CA2T(errorCode.c_str())));
					AfxMessageBox(errorMessage, MB_OK | MB_ICONERROR);
				}
			}
		}
		catch (const json::exception& e) {
			CString errorMessage;
			errorMessage.Format(_T("JSON parsing failed: %s"), (LPCTSTR)CString(CA2T(e.what())));
			AfxMessageBox(errorMessage, MB_OK | MB_ICONERROR);
		}
	}
	else {
		AfxMessageBox(_T("HttpSendRequest failed"));
	}

	// Clean up
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}

void CenzotechdeviceloginDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	RECT rect, rectDlg, translatedRect;
	this->GetWindowRect(&rectDlg);
	m_ctrlStaticLogo.GetWindowRect(&rect);
	m_ctrlStaticLogo.GetClientRect(&translatedRect);

	translatedRect.left = rect.left - rectDlg.left - 10;
	translatedRect.top = rect.top - rectDlg.top - 30;
	translatedRect.right = rect.right - rectDlg.left - 10;
	translatedRect.bottom = rect.bottom - rectDlg.top - 30;

	if ((translatedRect.left <= (point.x)) &&
		((point.x) <= translatedRect.right) &&
		((translatedRect.top) <= point.y) &&
		(point.y <= (translatedRect.bottom))) {
		SetCursor(LoadCursor(NULL, IDC_HAND));
	}
}

void CenzotechdeviceloginDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	RECT rect, rectDlg, translatedRect;
	this->GetWindowRect(&rectDlg);
	m_ctrlStaticLogo.GetWindowRect(&rect);
	m_ctrlStaticLogo.GetClientRect(&translatedRect);
	translatedRect.left = rect.left - rectDlg.left - 10;
	translatedRect.top = rect.top - rectDlg.top - 30;
	translatedRect.right = rect.right - rectDlg.left - 10;
	translatedRect.bottom = rect.bottom - rectDlg.top - 30;
	if ((translatedRect.left <= (point.x)) &&
		((point.x) <= translatedRect.right) &&
		((translatedRect.top) <= point.y) &&
		(point.y <= (translatedRect.bottom))) {
		ShellExecute(NULL, _T("open"), _T("https://enzotechcomputersolutions.com/timekeeping?page=1"), NULL,
			NULL, SW_SHOWNORMAL);
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CenzotechdeviceloginDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	m_bClickClose = true;  // signal thread to exit
	if (m_clockThread.joinable()) {
		m_clockThread.join();  // wait for thread to finish
	}
}

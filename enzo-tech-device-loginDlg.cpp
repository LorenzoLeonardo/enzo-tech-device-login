
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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString GetIsoTimestamp();
void SendPostJsonRequest(CString action, CString userId);
CString GetComputerNameMFC();
CString GetUsernameMFC();

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



CenzotechdeviceloginDlg::CenzotechdeviceloginDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ENZOTECHDEVICELOGIN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CenzotechdeviceloginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_LOGIN, m_ctrlBtnLogin);
	DDX_Control(pDX, IDC_BUTTON_LOGOUT, m_ctrlBtnLogout);
}

BEGIN_MESSAGE_MAP(CenzotechdeviceloginDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CenzotechdeviceloginDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT, &CenzotechdeviceloginDlg::OnBnClickedButtonLogout)
END_MESSAGE_MAP()


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

	// TODO: Add extra initialization here

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CenzotechdeviceloginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CenzotechdeviceloginDlg::OnBnClickedButtonLogin()
{
	SendPostJsonRequest(_T("login"), _T("<user>")); // Example user ID
}

void CenzotechdeviceloginDlg::OnBnClickedButtonLogout()
{
	SendPostJsonRequest(_T("logout"), _T("<user>")); // Example user ID
}

CString GetIsoTimestamp()
{
	SYSTEMTIME stLocal;
	GetLocalTime(&stLocal); // Local time for correct timezone offset

	// Simulate nanoseconds (7-digit fractional second)
	int microFraction = stLocal.wMilliseconds * 10000 + rand() % 10000; // Rough simulation of 7 digits

	// Get timezone offset
	TIME_ZONE_INFORMATION tzi;
	GetTimeZoneInformation(&tzi);

	int biasMinutes = -tzi.Bias;
	int hoursOffset = biasMinutes / 60;
	int minutesOffset = abs(biasMinutes % 60);

	CString iso;
	iso.Format(_T("%04d-%02d-%02dT%02d:%02d:%02d.%07d%+03d:%02d"),
		stLocal.wYear, stLocal.wMonth, stLocal.wDay,
		stLocal.wHour, stLocal.wMinute, stLocal.wSecond,
		microFraction, hoursOffset, minutesOffset);

	return iso;
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
		char buffer[4096];
		DWORD bytesRead = 0;
		CStringA response;

		while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
			buffer[bytesRead] = '\0';
			response += buffer;
		}

		AfxMessageBox(CA2T(response));
	}
	else {
		AfxMessageBox(_T("HttpSendRequest failed"));
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}

CString GetComputerNameMFC()
{
	TCHAR nameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(nameBuffer) / sizeof(nameBuffer[0]);

	if (GetComputerName(nameBuffer, &size)) {
		return CString(nameBuffer);
	}
	else {
		return _T("Unknown");
	}
}

CString GetUsernameMFC()
{
	TCHAR userName[UNLEN + 1];
	DWORD size = UNLEN + 1;

	if (GetUserName(userName, &size)) {
		return CString(userName);
	}
	else {
		return _T("UnknownUser");
	}
}
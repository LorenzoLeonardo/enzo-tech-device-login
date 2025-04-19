
// enzo-tech-device-loginDlg.cpp : implementation file
//
#include "pch.h"

#include "Settings.h"
#include "afxdialogex.h"
#include "enzo-tech-device-login.h"
#include "enzo-tech-device-loginDlg.h"
#include "framework.h"

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
#    define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx {
  public:
    CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

  protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation
  protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CenzotechdeviceloginDlg dialog

COLORREF DLG_BACKGROUND = RGB(13, 71, 161);
COLORREF GROUP_BACKGROUND = RGB(66, 165, 245);

CenzotechdeviceloginDlg::CenzotechdeviceloginDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_ENZOTECHDEVICELOGIN_DIALOG, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_brBackground.CreateSolidBrush(DLG_BACKGROUND);
    m_brGroupBox.CreateSolidBrush(GROUP_BACKGROUND);
    m_customClock.SetFontStyle(_T("Yu Gothic UI"));
    m_customClock.SetFontSize(28);
    m_customClock.SetFontWeight(FW_BOLD);
    m_customClock.SetTextColor(RGB(255, 255, 255));
    m_customClock.SetTextBKColor(DLG_BACKGROUND);
    m_customClock.CreateClock();
}
CenzotechdeviceloginDlg::~CenzotechdeviceloginDlg() {
    m_customClock.DestroyClock();
}

void CenzotechdeviceloginDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_LOGIN, m_ctrlBtnLogin);
    DDX_Control(pDX, IDC_BUTTON_LOGOUT, m_ctrlBtnLogout);
    DDX_Control(pDX, IDC_MY_GROUPBOX, m_myGroupBox);
    DDX_Control(pDX, IDC_STATIC_PICTURE, m_ctrlStaticLogo); // associate your static control
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
    CRect rect;

    // Get the DPI scaling for this DC
    int dpiX = GetDeviceCaps(cdc.GetSafeHdc(), LOGPIXELSX);
    int dpiY = GetDeviceCaps(cdc.GetSafeHdc(), LOGPIXELSY);
    double scaleX = dpiX / 96.0;
    double scaleY = dpiY / 96.0;

    // Get the group box position
    GetDlgItem(IDC_MY_GROUPBOX)->GetWindowRect(&rect);
    ScreenToClient(&rect);

    // Apply scaling to clock position
    int x = static_cast<int>((rect.left / scaleX));
    int y = static_cast<int>((rect.left / scaleY)); // relative to the top of the dialog

    m_customClock.DrawClock(&cdc, x, y);
}
void CenzotechdeviceloginDlg::ClockThread() {
    while (!m_bClickClose) {
        UpdateClock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// CenzotechdeviceloginDlg message handlers

BOOL CenzotechdeviceloginDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);  // Set big icon
    SetIcon(m_hIcon, FALSE); // Set small icon

    CString path = GetIniFilePath(_T("user.ini"));
    CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
    CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
    CString prev_action = ReadIniValue(_T("User"), _T("action"), _T("logout"), path);
    CString device_id = GetComputerNameMFC();
    CString server_version = ReadIniValue(_T("User"), _T("Serverversion"), _T("Unknown"), path);
    SetDlgItemText(IDC_EDIT_NAME, name);
    SetDlgItemText(IDC_EDIT_EMAIL, email);
    SetDlgItemText(IDC_EDIT_DEVICE_ID, device_id);

    CString footer;
    footer.Format(_T("© 2025 Enzo Tech Computer Solutions\r\nEnzo Tech Webserver version %s"),
                  server_version.GetString());
    SetDlgItemText(IDC_STATIC_FOOTER, footer);

    if (prev_action == _T("login")) {
        m_ctrlBtnLogin.EnableWindow(FALSE);
        m_ctrlBtnLogout.EnableWindow(TRUE);
    } else {
        m_ctrlBtnLogin.EnableWindow(TRUE);
        m_ctrlBtnLogout.EnableWindow(FALSE);
    }
    m_ctrlStaticLogo.SetBitmapResource(IDB_BITMAP1);
    m_clockThread = std::thread(&CenzotechdeviceloginDlg::ClockThread, this);
    return TRUE; // return TRUE  unless you set the focus to a control
}

void CenzotechdeviceloginDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CenzotechdeviceloginDlg::OnPaint() {
    if (IsIconic()) {
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
    } else {
        CPaintDC dc(this); // device context for painting

        // Call base class (important!)
        CDialogEx::OnPaint();

        // Get the group box control
        CWnd* pGroup = GetDlgItem(IDC_MY_GROUPBOX);
        if (pGroup) {
            CRect rect;
            pGroup->GetWindowRect(&rect);

            int dpiY = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY);
            double scaleY = dpiY / 96.0;
            int y = static_cast<int>((10 * scaleY));

            rect.top += y;
            ScreenToClient(&rect);

            dc.FillSolidRect(&rect, RGB(66, 165, 245));
            CDialogEx::OnPaint();
        }
    }
}

HBRUSH CenzotechdeviceloginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    // Set dialog background color
    if (nCtlColor == CTLCOLOR_DLG) {
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }

    if (pWnd->GetDlgCtrlID() == IDC_STATIC_FOOTER) {
        pDC->SetTextColor(RGB(21, 101, 192));
        pDC->SetBkMode(TRANSPARENT);
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }

    else if (pWnd->GetDlgCtrlID() == IDC_STATIC_NAME || pWnd->GetDlgCtrlID() == IDC_STATIC_EMAIL ||
             pWnd->GetDlgCtrlID() == IDC_STATIC_DEVICE || pWnd->GetDlgCtrlID() == IDC_MY_GROUPBOX) {
        pDC->SetTextColor(RGB(255, 255, 255));
        pDC->SetBkMode(TRANSPARENT);
        return (HBRUSH)m_brGroupBox.GetSafeHandle();
    }
    return hbr;
}

BOOL CenzotechdeviceloginDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
    // TODO: Add your message handler code here and/or call default
    if (pWnd->GetDlgCtrlID() == IDC_BUTTON_LOGIN || pWnd->GetDlgCtrlID() == IDC_BUTTON_LOGOUT) {
        ::SetCursor(::LoadCursor(NULL, IDC_HAND));
        return TRUE;
    }
    return CDialogEx::OnSetCursor(pWnd, nHitTest, message);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CenzotechdeviceloginDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}

void CenzotechdeviceloginDlg::OnBnClickedButtonLogin() {
    CString path = GetIniFilePath(_T("user.ini"));
    CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
    CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
    CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
    CString device_id = GetComputerNameMFC();
    CString username = GetUsernameMFC();
    CString timestamp = GetIsoTimestamp();
    CString action = _T("login");

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
            EndDialog(IDOK);
        }
        m_ctrlBtnLogin.EnableWindow(FALSE);
        m_ctrlBtnLogout.EnableWindow(TRUE);

        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Badge IN successful"), _T("Information"),
                     MB_OK | MB_ICONINFORMATION);
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
                EndDialog(IDOK);
            }
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
                         _T("Session has expired. Please run the program again."),
                         _T("Information"), MB_OK | MB_ICONERROR);
        } else {
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Server Error. Please try again."),
                         _T("Information"), MB_OK | MB_ICONERROR);
        }
        EndDialog(IDOK);
    } else if (std::holds_alternative<HttpError>(resp)) {
        HttpError response = std::get<HttpError>(resp);

        CString error(response.http_error.c_str());
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), error.GetString(), _T("Information"),
                     MB_OK | MB_ICONERROR);
    } else {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Unknown error"), _T("Information"),
                     MB_OK | MB_ICONERROR);
    }
}

void CenzotechdeviceloginDlg::OnBnClickedButtonLogout() {
    CString path = GetIniFilePath(_T("user.ini"));
    CString session_id = ReadIniValue(_T("User"), _T("session_id"), _T("default_session_id"), path);
    CString user_id = ReadIniValue(_T("User"), _T("user_id"), _T("default_user_id"), path);
    CString name = ReadIniValue(_T("User"), _T("name"), _T("default_name"), path);
    CString email = ReadIniValue(_T("User"), _T("email"), _T("default_email"), path);
    CString device_id = GetComputerNameMFC();
    CString username = GetUsernameMFC();
    CString timestamp = GetIsoTimestamp();
    CString action = _T("logout");

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
            EndDialog(IDOK);
        }
        m_ctrlBtnLogin.EnableWindow(TRUE);
        m_ctrlBtnLogout.EnableWindow(FALSE);

        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Badge OUT successful"), _T("Information"),
                     MB_OK | MB_ICONINFORMATION);
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
                EndDialog(IDOK);
            }
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(),
                         _T("Session has expired. Please run the program again."),
                         _T("Information"), MB_OK | MB_ICONERROR);
        } else {
            ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Server Error. Please try again."),
                         _T("Information"), MB_OK | MB_ICONERROR);
        }
        EndDialog(IDOK);
    } else if (std::holds_alternative<HttpError>(resp)) {
        HttpError response = std::get<HttpError>(resp);

        CString error(response.http_error.c_str());
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), error.GetString(), _T("Information"),
                     MB_OK | MB_ICONERROR);
    } else {
        ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), _T("Unknown error"), _T("Information"),
                     MB_OK | MB_ICONERROR);
    }
}

void CenzotechdeviceloginDlg::OnMouseMove(UINT nFlags, CPoint point) {
    CRect rect;
    m_ctrlStaticLogo.GetWindowRect(&rect); // Physical screen coordinates
    ScreenToClient(&rect);                 // Physical client coordinates

    CPaintDC dc(this);

    int dpiX = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSX);
    int dpiY = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY);
    float scaleX = dpiX / 96.0f;
    float scaleY = dpiY / 96.0f;

    // Control size in logical pixels
    int logicalWidth = static_cast<int>(rect.Width() * scaleX);
    int logicalHeight = static_cast<int>(rect.Height() * scaleY);

    rect.right = rect.left + logicalWidth;
    rect.bottom = rect.top + logicalHeight;
    if (rect.PtInRect(point)) {
        SetCursor(LoadCursor(NULL, IDC_HAND));
    }

    CDialogEx::OnMouseMove(nFlags, point);
}

void CenzotechdeviceloginDlg::OnLButtonDown(UINT nFlags, CPoint point) {
    CRect rect;
    m_ctrlStaticLogo.GetWindowRect(&rect); // Physical screen coordinates
    ScreenToClient(&rect);                 // Physical client coordinates

    CPaintDC dc(this);

    int dpiX = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSX);
    int dpiY = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY);
    float scaleX = dpiX / 96.0f;
    float scaleY = dpiY / 96.0f;

    // Control size in logical pixels
    int logicalWidth = static_cast<int>(rect.Width() * scaleX);
    int logicalHeight = static_cast<int>(rect.Height() * scaleY);

    rect.right = rect.left + logicalWidth;
    rect.bottom = rect.top + logicalHeight;
    if (rect.PtInRect(point)) { // point is also in physical client coords
        ShellExecute(NULL, _T("open"),
                     Settings::GetInstance().Url() + _T("/applications/timekeeping?page=1"), NULL,
                     NULL, SW_SHOWNORMAL);
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CenzotechdeviceloginDlg::OnDestroy() {
    CDialogEx::OnDestroy();

    // TODO: Add your message handler code here
    m_bClickClose = true; // signal thread to exit
    if (m_clockThread.joinable()) {
        m_clockThread.join(); // wait for thread to finish
    }
}

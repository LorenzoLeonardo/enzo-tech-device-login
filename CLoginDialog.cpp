// CLoginDialog.cpp : implementation file
//

#include "pch.h"

#include "CLoginDialog.h"
#include "afxdialogex.h"

#include "Definitions.h"
#include "enzo-tech-device-login.h"

// CLoginDialog dialog

IMPLEMENT_DYNAMIC(CLoginDialog, CDialogEx)

CLoginDialog::CLoginDialog(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_LOGIN, pParent) {
    m_brBackground.CreateSolidBrush(DLG_BACKGROUND_COLOR);
    m_brGroupBox.CreateSolidBrush(GROUP_BACKGROUND);
}

CLoginDialog::~CLoginDialog() {}

void CLoginDialog::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
ON_BN_CLICKED(IDC_BUTTON_GOOGLE, &CLoginDialog::OnBnClickedButtonGoogle)
ON_BN_CLICKED(IDC_BUTTON_MICROSOFT, &CLoginDialog::OnBnClickedButtonMicrosoft)
ON_WM_PAINT()
ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CLoginDialog message handlers
BOOL CLoginDialog::OnInitDialog() {
    CDialogEx::OnInitDialog();

    m_GoogleButton.SubclassDlgItem(IDC_BUTTON_GOOGLE, this);
    m_GoogleButton.SetBitmapFromResource(IDB_BITMAP_GOOGLE);
    m_GoogleButton.SetBackgroundColor(COLOR_WHITE);

    m_MicrosoftButton.SubclassDlgItem(IDC_BUTTON_MICROSOFT, this);
    m_MicrosoftButton.SetBitmapFromResource(IDB_BITMAP_MICROSOFT);
    m_MicrosoftButton.SetBackgroundColor(COLOR_VERY_DARK_GRAY);

    return TRUE;
}
void CLoginDialog::OnBnClickedButtonGoogle() {
    // TODO: Add your control notification handler code here
    m_provider = Provider::Google;
    OnOK();
}

void CLoginDialog::OnBnClickedButtonMicrosoft() {
    // TODO: Add your control notification handler code here
    m_provider = Provider::Microsoft;
    OnOK();
}

void CLoginDialog::OnPaint() {
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

        dc.FillSolidRect(&rect, GROUP_BACKGROUND);

        // Create "Segoe UI Black", 12pt font
        CFont font;
        font.CreatePointFont(static_cast<int>(120 * scaleY), _T("Segoe UI Black")); // 12pt
        CFont* pOldFont = dc.SelectObject(&font);

        // Set text color and transparent background
        dc.SetTextColor(COLOR_WHITE); // White
        dc.SetBkMode(TRANSPARENT);

        // Draw text at top-left of the rect
        dc.TextOut(rect.left + 8, rect.top + 4, _T("Authenticate User"));

        // Restore old font
        dc.SelectObject(pOldFont);
        font.DeleteObject();
        CDialogEx::OnPaint();
    }
}

HBRUSH CLoginDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_DLG) {
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }

    if (pWnd->GetDlgCtrlID() == IDC_MY_GROUPBOX) {
        pDC->SetTextColor(COLOR_WHITE);
        return (HBRUSH)m_brGroupBox.GetSafeHandle();
    }

    return hbr;
}

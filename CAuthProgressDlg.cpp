// CAuthProgressDlg.cpp : implementation file
//

#include "pch.h"

#include "CAuthProgressDlg.h"
#include "Settings.h"
#include "afxdialogex.h"
#include "enzo-tech-device-login.h"
// CAuthProgressDlg dialog

IMPLEMENT_DYNAMIC(CAuthProgressDlg, CDialogEx)

CAuthProgressDlg::CAuthProgressDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_AUTH_PROGRESS, pParent), m_Frame(0) {
    m_brBackground.CreateSolidBrush(RGB(13, 71, 161));
    m_dotBrush.CreateSolidBrush(RGB(255, 255, 255));
}

CAuthProgressDlg::~CAuthProgressDlg() {}

void CAuthProgressDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAuthProgressDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_TIMER()
ON_BN_CLICKED(IDCANCEL, &CAuthProgressDlg::OnBnClickedCancel)
ON_WM_DESTROY()
ON_WM_CTLCOLOR()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CAuthProgressDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Add styles to reduce flicker
    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    SetWindowLong(m_hWnd, GWL_STYLE, style);

    SetTimer(1, 50, nullptr); // 50ms interval for smooth rotation
    this->SetWindowText(_T("Connecting to ") + Settings::GetInstance().Url());
    return TRUE;
}

void CAuthProgressDlg::OnPaint() {
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);

    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

    // Fill background
    memDC.FillSolidRect(&rect, RGB(13, 71, 161));

    const int dotCount = 10;
    const int radius = 4;
    const int orbitRadius = 20;
    const double angleStep = 2 * 3.14159265 / dotCount;
    CPoint center(rect.Width() / 2, rect.Height() / 2);

    for (int i = 0; i < dotCount; ++i) {
        double angle = angleStep * ((i + m_Frame) % dotCount);
        int x = static_cast<int>(center.x + orbitRadius * cos(angle));
        int y = static_cast<int>(center.y + orbitRadius * sin(angle));

        int r = (i == 0) ? radius + 2 : radius;
        CBrush* pOldBrush = memDC.SelectObject(&m_dotBrush);
        memDC.SetBkMode(TRANSPARENT);
        memDC.Ellipse(x - r, y - r, x + r, y + r);
        memDC.SelectObject(pOldBrush);
    }

    // Blit to screen
    dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(pOldBitmap);
}

void CAuthProgressDlg::OnTimer(UINT_PTR nIDEvent) {
    m_Frame = (m_Frame + 1) % 10;
    RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
    CDialogEx::OnTimer(nIDEvent);
}

void CAuthProgressDlg::OnBnClickedCancel() {
    // TODO: Add your control notification handler code here
    m_hasCancelled = true;
    CDialogEx::OnCancel();
}

void CAuthProgressDlg::OnDestroy() {
    m_hasCancelled = true;
    KillTimer(1);
    CDialogEx::OnDestroy();
}

HBRUSH CAuthProgressDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    // Set dialog background color
    if (nCtlColor == CTLCOLOR_DLG) {
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }
    if (pWnd->GetDlgCtrlID() == IDC_STATIC_AUTHENTICATING_LABEL) {
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(RGB(255, 255, 255));
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }
    return hbr;
}

BOOL CAuthProgressDlg::OnEraseBkgnd(CDC* pDC) {
    return TRUE; // Prevent background erase to reduce flicker
}

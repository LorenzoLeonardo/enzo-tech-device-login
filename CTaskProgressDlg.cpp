// CTaskProgressDlg.cpp : implementation file
//

#include "pch.h"

#include "CTaskProgressDlg.h"
#include "Definitions.h"
#include "Settings.h"
#include "afxdialogex.h"
#include "enzo-tech-device-login.h"

// CTaskProgressDlg dialog
#include <gdiplus.h>
using namespace Gdiplus;

#pragma comment(lib, "gdiplus.lib")

IMPLEMENT_DYNAMIC(CTaskProgressDlg, CDialogEx)

CTaskProgressDlg::CTaskProgressDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_AUTH_PROGRESS, pParent), m_Frame(0) {
    m_brBackground.CreateSolidBrush(DLG_BACKGROUND_COLOR);
    m_dotBrush.CreateSolidBrush(COLOR_WHITE);
}

CTaskProgressDlg::~CTaskProgressDlg() {}

void CTaskProgressDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTaskProgressDlg, CDialogEx)
ON_WM_PAINT()
ON_WM_TIMER()
ON_BN_CLICKED(IDCANCEL, &CTaskProgressDlg::OnBnClickedCancel)
ON_WM_DESTROY()
ON_WM_CTLCOLOR()
ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CTaskProgressDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    CenterWindow(AfxGetMainWnd());

    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr);

    // Add styles to reduce flicker
    LONG style = GetWindowLong(m_hWnd, GWL_STYLE);
    style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    SetWindowLong(m_hWnd, GWL_STYLE, style);

    SetTimer(1, 25, nullptr); // 25ms interval for smooth rotation
    return TRUE;
}

void CTaskProgressDlg::OnPaint() {
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    // Create a memory bitmap and graphics context for double buffering
    Bitmap memBitmap(rect.Width(), rect.Height(), PixelFormat32bppARGB);
    Graphics memGraphics(&memBitmap);

    // Set high-quality rendering
    memGraphics.SetSmoothingMode(SmoothingModeAntiAlias);
    memGraphics.SetCompositingQuality(CompositingQualityHighQuality);
    memGraphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // Clear background with a solid color
    SolidBrush backgroundBrush(Color(255, 13, 71, 161)); // Blue background
    memGraphics.FillRectangle(&backgroundBrush, 0, 0, rect.Width(), rect.Height());

    const int dotCount = 12;
    const int radius = 4;
    const int orbitRadius = 25;
    const double angleStep = 2 * 3.14159265 / dotCount;
    CPoint center(rect.Width() / 2, rect.Height() / 2);

    // Loop through each dot and apply fading
    for (int i = 0; i < dotCount; ++i) {
        int index = (i + m_Frame) % dotCount;
        double angle = angleStep * index;
        int x = static_cast<int>(center.x + orbitRadius * cos(angle));
        int y = static_cast<int>(center.y + orbitRadius * sin(angle));

        // Fade effect: make each dot fade in and out more gradually
        // Use a sine function for smooth fading
        double fadeFactor = (sin(m_Frame * 0.1 + i * 0.5) + 1) / 2; // Oscillates between 0 and 1
        int alpha = static_cast<int>(255 * fadeFactor); // Apply fade based on sine oscillation

        Color color(alpha, 255, 255, 255); // White color with fading alpha
        SolidBrush brush(color);

        // Draw the dot with fading effect
        memGraphics.FillEllipse(&brush, x - radius, y - radius, radius * 2, radius * 2);
    }

    // Blit from memory bitmap to the screen
    Graphics screenGraphics(dc);
    screenGraphics.DrawImage(&memBitmap, 0, 0);
}

void CTaskProgressDlg::OnTimer(UINT_PTR nIDEvent) {
    if (nIDEvent == 1) {
        m_Frame = (m_Frame + 1) % 1000;
        Invalidate(FALSE);
    }
    CDialogEx::OnTimer(nIDEvent);
}

void CTaskProgressDlg::OnBnClickedCancel() {
    // TODO: Add your control notification handler code here
    m_hasCancelled = true;
    CDialogEx::OnCancel();
}

void CTaskProgressDlg::OnDestroy() {
    m_hasCancelled = true;
    KillTimer(1);
    CDialogEx::OnDestroy();
}

HBRUSH CTaskProgressDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    // Set dialog background color
    if (nCtlColor == CTLCOLOR_DLG) {
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }
    if (pWnd->GetDlgCtrlID() == IDC_STATIC_AUTHENTICATING_LABEL) {
        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(COLOR_WHITE);
        return (HBRUSH)m_brBackground.GetSafeHandle();
    }
    return hbr;
}

BOOL CTaskProgressDlg::OnEraseBkgnd(CDC* pDC) {
    return TRUE; // Prevent background erase to reduce flicker
}

void CTaskProgressDlg::PostNcDestroy() {
    // GDI+ shutdown
    GdiplusShutdown(m_gdiplusToken);
    CDialogEx::PostNcDestroy();
}

void CTaskProgressDlg::SetBodyText(LPCTSTR text) {
    SetDlgItemText(IDC_STATIC_AUTHENTICATING_LABEL, text);
}
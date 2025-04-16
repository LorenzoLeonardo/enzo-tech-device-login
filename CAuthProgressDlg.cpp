// CAuthProgressDlg.cpp : implementation file
//

#include "pch.h"
#include "enzo-tech-device-login.h"
#include "afxdialogex.h"
#include "CAuthProgressDlg.h"


// CAuthProgressDlg dialog

IMPLEMENT_DYNAMIC(CAuthProgressDlg, CDialogEx)

CAuthProgressDlg::CAuthProgressDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AUTH_PROGRESS, pParent), m_Frame(0)
{

}

CAuthProgressDlg::~CAuthProgressDlg()
{
}

void CAuthProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAuthProgressDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_TIMER()
    ON_BN_CLICKED(IDCANCEL, &CAuthProgressDlg::OnBnClickedCancel)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CAuthProgressDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetTimer(1, 100, nullptr);  // 100ms interval for smooth rotation
    return TRUE;
}

void CAuthProgressDlg::OnPaint()
{
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    const int dotCount = 5;
    const int spacing = 20;
    const int radius = 5;

    int startX = rect.Width() / 2 - ((dotCount - 1) * spacing) / 2;
    int y = rect.Height() / 2;

    for (int i = 0; i < dotCount; ++i)
    {
        int alpha = (i == m_Frame) ? 255 : 100;
        int r = (i == m_Frame) ? radius + 2 : radius;

        COLORREF color = RGB(0, 0, 0);
        CBrush brush;
        brush.CreateSolidBrush(color);
        CBrush* pOldBrush = dc.SelectObject(&brush);
        dc.SetBkMode(TRANSPARENT);

        int x = startX + i * spacing;
        dc.Ellipse(x - r, y - r, x + r, y + r);

        dc.SelectObject(pOldBrush);
    }
}

void CAuthProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
    m_Frame = (m_Frame + 1) % 5;
    Invalidate();
    CDialogEx::OnTimer(nIDEvent);
}
void CAuthProgressDlg::OnBnClickedCancel()
{
    // TODO: Add your control notification handler code here
    m_hasCancelled = true;
    CDialogEx::OnCancel();
}
void CAuthProgressDlg::OnDestroy()
{
    m_hasCancelled = true;
    KillTimer(1);
    CDialogEx::OnDestroy();
}
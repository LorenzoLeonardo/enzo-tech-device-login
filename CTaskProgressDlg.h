#pragma once
#include "afxdialogex.h"

#include <gdiplus.h>
using namespace Gdiplus;
// CTaskProgressDlg dialog

class CTaskProgressDlg : public CDialogEx {
    DECLARE_DYNAMIC(CTaskProgressDlg)

  public:
    CTaskProgressDlg(CWnd* pParent = nullptr); // standard constructor
    virtual ~CTaskProgressDlg();

    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();

  private:
    int m_Frame;
    bool m_hasCancelled = false;
    CBrush m_brBackground;
    CBrush m_dotBrush;
    ULONG_PTR m_gdiplusToken;
    CString m_bodyText;
// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_AUTH_PROGRESS };
#endif

  protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual void PostNcDestroy();
    DECLARE_MESSAGE_MAP()

  public:
    afx_msg void OnBnClickedCancel();
    bool HasCancelled() const { return m_hasCancelled; }
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    void SetBodyText(LPCTSTR text);
};

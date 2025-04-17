#pragma once
#include "afxdialogex.h"

// CAuthProgressDlg dialog

class CAuthProgressDlg : public CDialogEx {
    DECLARE_DYNAMIC(CAuthProgressDlg)

  public:
    CAuthProgressDlg(CWnd* pParent = nullptr); // standard constructor
    virtual ~CAuthProgressDlg();

    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();

  private:
    int    m_Frame; // Current frame in animation (0 to 4)
    bool   m_hasCancelled = false;
    CBrush m_brBackground;
// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_AUTH_PROGRESS };
#endif

  protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

  public:
    afx_msg void   OnBnClickedCancel();
    bool           HasCancelled() { return m_hasCancelled; }
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

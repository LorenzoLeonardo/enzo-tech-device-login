
// enzo-tech-device-loginDlg.h : header file
//

#pragma once


// CenzotechdeviceloginDlg dialog
class CenzotechdeviceloginDlg : public CDialogEx
{
// Construction
public:
	CenzotechdeviceloginDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ENZOTECHDEVICELOGIN_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CBrush m_brBackground;
	CBrush m_brGroupBox;

	CButton m_myGroupBox;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton m_ctrlBtnLogin;
	CButton m_ctrlBtnLogout;
	afx_msg void OnBnClickedButtonLogin();
	afx_msg void OnBnClickedButtonLogout();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CStatic m_ctrlStaticLogo;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};

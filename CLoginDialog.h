#pragma once
#include "afxdialogex.h"

#include "CLogoButton.h"

// CLoginDialog dialog

class CLoginDialog : public CDialogEx {
    DECLARE_DYNAMIC(CLoginDialog)

  public:
    CLoginDialog(CWnd* pParent = nullptr); // standard constructor
    virtual ~CLoginDialog();

    CLogoButton m_GoogleButton;
    CLogoButton m_MicrosoftButton;

    virtual BOOL OnInitDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_DIALOG_LOGIN };
#endif

  protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()
  public:
    afx_msg void OnBnClickedButtonGoogle();
    afx_msg void OnBnClickedButtonMicrosoft();
};

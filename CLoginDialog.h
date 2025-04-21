#pragma once
#include "afxdialogex.h"

#include "CLogoButton.h"

enum class Provider { Google, Microsoft };

class Providers {
  public:
    static CString ToString(Provider provider) {
        switch (provider) {
            case Provider::Google:
                return _T("Google");
            case Provider::Microsoft:
                return _T("Microsoft");
            default:
                return _T("Unknown");
        }
    }

    static Provider FromString(const CString& str) {
        if (str.CompareNoCase(_T("Google")) == 0)
            return Provider::Google;
        if (str.CompareNoCase(_T("Microsoft")) == 0)
            return Provider::Microsoft;
        FatalAppExit(0, _T("panic in FromString, Not valid string!"));
    }
};

class CLoginDialog : public CDialogEx {
    DECLARE_DYNAMIC(CLoginDialog)

  public:
    CLoginDialog(CWnd* pParent = nullptr); // standard constructor
    virtual ~CLoginDialog();

    CLogoButton m_GoogleButton;
    CLogoButton m_MicrosoftButton;
    Provider m_provider = {};

    virtual BOOL OnInitDialog();
    Provider Provider() { return std::move(m_provider); }

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

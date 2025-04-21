// CLoginDialog.cpp : implementation file
//

#include "pch.h"

#include "CLoginDialog.h"
#include "afxdialogex.h"
#include "enzo-tech-device-login.h"

// CLoginDialog dialog

IMPLEMENT_DYNAMIC(CLoginDialog, CDialogEx)

CLoginDialog::CLoginDialog(CWnd* pParent /*=nullptr*/) : CDialogEx(IDD_DIALOG_LOGIN, pParent) {}

CLoginDialog::~CLoginDialog() {}

void CLoginDialog::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
ON_BN_CLICKED(IDC_BUTTON_GOOGLE, &CLoginDialog::OnBnClickedButtonGoogle)
ON_BN_CLICKED(IDC_BUTTON_MICROSOFT, &CLoginDialog::OnBnClickedButtonMicrosoft)
END_MESSAGE_MAP()

// CLoginDialog message handlers
BOOL CLoginDialog::OnInitDialog() {
    CDialogEx::OnInitDialog();

    m_GoogleButton.SubclassDlgItem(IDC_BUTTON_GOOGLE, this);
    m_GoogleButton.SetBitmapFromResource(IDB_BITMAP_GOOGLE);
    m_GoogleButton.SetBackgroundColor(RGB(255, 255, 255));

    m_MicrosoftButton.SubclassDlgItem(IDC_BUTTON_MICROSOFT, this);
    m_MicrosoftButton.SetBitmapFromResource(IDB_BITMAP_MICROSOFT);
    m_MicrosoftButton.SetBackgroundColor(RGB(47, 47, 47));

    return TRUE;
}
void CLoginDialog::OnBnClickedButtonGoogle() {
    // TODO: Add your control notification handler code here
}

void CLoginDialog::OnBnClickedButtonMicrosoft() {
    // TODO: Add your control notification handler code here
}

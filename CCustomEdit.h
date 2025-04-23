#pragma once
#include <afxwin.h>

class CCustomEdit : public CEdit {
  public:
    CCustomEdit();
    virtual ~CCustomEdit();

  protected:
    afx_msg void OnPaint();
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()

  private:
    CBrush m_brBackground;
};

#pragma once
#include "Definitions.h"

class CLogoButton : public CButton {
  public:
    CBitmap m_Bitmap;
    CString m_Text;
    COLORREF m_BackgroundColor = COLOR_WHITE;
    COLORREF m_TextColor = RGB(0, 0, 0);
    bool m_bHover = false;

    void SetBitmapFromResource(UINT nIDResource);
    void SetText(CString text);
    void SetBackgroundColor(COLORREF color);
    void SetTextColor(COLORREF color);

  protected:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

  public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
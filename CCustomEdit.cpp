#include "pch.h"

#include "CCustomEdit.h"


CCustomEdit::CCustomEdit() {
    m_brBackground.CreateSolidBrush(RGB(255, 255, 255)); // Light gray background
}

CCustomEdit::~CCustomEdit() {}

BEGIN_MESSAGE_MAP(CCustomEdit, CEdit)
ON_WM_PAINT()
ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

void CCustomEdit::OnPaint() {
    CPaintDC dc(this); // Device context for painting
    CRect rect;
    GetClientRect(&rect);

    CString strText;
    GetWindowText(strText);

    // Fill background
    dc.FillRect(&rect, &m_brBackground);

    // Set text attributes
    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(RGB(128, 128, 128));

    // Get current font
    CFont* pOldFont = dc.SelectObject(GetFont());

    // Measure text height
    CSize textSize = dc.GetTextExtent(strText);
    int y = (rect.Height() - textSize.cy) / 2;

    // Apply margin (e.g., 10px left margin)
    CRect textRect = rect;
    const int leftMargin = 5;
    textRect.left += leftMargin;
    textRect.top = y;

    // Draw text
    dc.DrawText(strText, &textRect, DT_LEFT | DT_SINGLELINE);

    dc.SelectObject(pOldFont);
}

HBRUSH CCustomEdit::CtlColor(CDC* pDC, UINT nCtlColor) {
    // Keep transparency mode to avoid flicker on some systems
    pDC->SetBkMode(TRANSPARENT);
    return (HBRUSH)m_brBackground.GetSafeHandle();
}
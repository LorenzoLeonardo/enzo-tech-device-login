#include "pch.h"

#include "CLogoButton.h"

void CLogoButton::SetBitmapFromResource(UINT nIDResource) {
    m_Bitmap.DeleteObject();
    m_Bitmap.LoadBitmap(nIDResource);
}

void CLogoButton::SetText(CString text) {
    m_Text = text;
}

void CLogoButton::SetBackgroundColor(COLORREF color) {
    m_BackgroundColor = color;
}

void CLogoButton::SetTextColor(COLORREF color) {
    m_TextColor = color;
}

void CLogoButton::DrawItem(LPDRAWITEMSTRUCT lpDIS) {
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);
    CRect rect = lpDIS->rcItem;
    int padding = 0;
    int imageWidth = 0;

    pDC->FillSolidRect(&rect, m_BackgroundColor);

    if (m_Bitmap.m_hObject != nullptr) {
        CDC memDC;
        if (memDC.CreateCompatibleDC(pDC)) {
            CBitmap* pOldBmp = memDC.SelectObject(&m_Bitmap);

            BITMAP bmp;
            m_Bitmap.GetBitmap(&bmp);

            // Resize logic
            int targetHeight = rect.Height(); // leave vertical margin
            int targetWidth = (bmp.bmWidth * targetHeight) / bmp.bmHeight;
            imageWidth = targetWidth;

            int imgLeft = rect.left;
            int imgTop = rect.top + (rect.Height() - targetHeight) / 2;

            // StretchBlt with proper resizing
            pDC->SetStretchBltMode(HALFTONE); // Better quality
            pDC->StretchBlt(imgLeft, imgTop, targetWidth, targetHeight, &memDC, 0, 0, bmp.bmWidth,
                            bmp.bmHeight, SRCCOPY);

            memDC.SelectObject(pOldBmp);
            memDC.DeleteDC();
        }
    }

    if (m_bHover) {
        COLORREF borderColor = (~m_BackgroundColor) & 0x00FFFFFF;
        CPen pen(PS_DOT, 1, borderColor);
        CPen* pOldPen = pDC->SelectObject(&pen);
        HBRUSH hOldBrush = (HBRUSH)pDC->SelectObject(GetStockObject(NULL_BRUSH)); // Fix here
        CRect innerRect = rect;

        innerRect.InflateRect(-1, -1); // Shrink so the 1px border fits within visible bounds
        pDC->Rectangle(&innerRect);
        pDC->SelectObject(hOldBrush); // Restore old brush
        pDC->SelectObject(pOldPen);   // Restore old pen
    }

    if (lpDIS->itemState & ODS_FOCUS)
        pDC->DrawFocusRect(&rect);
}

BEGIN_MESSAGE_MAP(CLogoButton, CButton)
ON_WM_MOUSEMOVE()
ON_WM_MOUSELEAVE()
ON_WM_ERASEBKGND()
ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void CLogoButton::OnMouseMove(UINT nFlags, CPoint point) {
    if (!m_bHover) {
        m_bHover = true;

        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = GetSafeHwnd();
        tme.dwHoverTime = 1;
        TrackMouseEvent(&tme);

        Invalidate(); // trigger redraw
    }

    CButton::OnMouseMove(nFlags, point);
}

void CLogoButton::OnMouseLeave() {
    m_bHover = false;
    Invalidate(); // redraw when mouse leaves
}

BOOL CLogoButton::OnEraseBkgnd(CDC* pDC) {
    // prevent flicker
    return TRUE;
}

BOOL CLogoButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
    ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
    return TRUE; // we handled the cursor
}

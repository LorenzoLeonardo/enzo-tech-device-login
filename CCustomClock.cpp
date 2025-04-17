#include "pch.h"

#include "CCustomClock.h"

CCustomClock* pCCustomClock;
CCustomClock::CCustomClock() {
    m_csFontStyle = _T("Microsoft Sans Serif");
    m_nFontSize   = 12;
    m_nFontWeight = FW_NORMAL;
    m_textColor   = RGB(0, 0, 0);
    m_textBKColor = RGB(13, 71, 161);
    pCCustomClock = this;
}
CCustomClock::~CCustomClock() {}

void CCustomClock::CreateClock() {
    // Get DPI from screen DC
    HDC hScreenDC = ::GetDC(NULL);
    int dpiY      = GetDeviceCaps(hScreenDC, LOGPIXELSY);
    ::ReleaseDC(NULL, hScreenDC);

    // Convert point size to logical font height
    int logicalFontHeight     = -MulDiv(m_nFontSize, dpiY, 72);
    int logicalFontHeightAMPM = -MulDiv(m_nFontSize / 2, dpiY, 72);
    int logicalFontHeightDate = -MulDiv(m_nFontSize / 3, dpiY, 72);

    VERIFY(m_cfont.CreateFont(logicalFontHeight,        // nHeight
                              0,                        // nWidth
                              0,                        // nEscapement
                              0,                        // nOrientation
                              m_nFontWeight,            // nWeight
                              FALSE,                    // bItalic
                              FALSE,                    // bUnderline
                              0,                        // cStrikeOut
                              ANSI_CHARSET,             // nCharSet
                              OUT_DEFAULT_PRECIS,       // nOutPrecision
                              CLIP_DEFAULT_PRECIS,      // nClipPrecision
                              DEFAULT_QUALITY,          // nQuality
                              DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
                              m_csFontStyle));          // lpszFacename

    VERIFY(m_cFontAMPM.CreateFont(logicalFontHeightAMPM,    // nHeight
                                  0,                        // nWidth
                                  0,                        // nEscapement
                                  0,                        // nOrientation
                                  m_nFontWeight,            // nWeight
                                  FALSE,                    // bItalic
                                  FALSE,                    // bUnderline
                                  0,                        // cStrikeOut
                                  ANSI_CHARSET,             // nCharSet
                                  OUT_DEFAULT_PRECIS,       // nOutPrecision
                                  CLIP_DEFAULT_PRECIS,      // nClipPrecision
                                  DEFAULT_QUALITY,          // nQuality
                                  DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
                                  m_csFontStyle));

    VERIFY(m_cFontDate.CreateFont(logicalFontHeightDate,    // nHeight
                                  0,                        // nWidth
                                  0,                        // nEscapement
                                  0,                        // nOrientation
                                  m_nFontWeight,            // nWeight
                                  FALSE,                    // bItalic
                                  FALSE,                    // bUnderline
                                  0,                        // cStrikeOut
                                  ANSI_CHARSET,             // nCharSet
                                  OUT_DEFAULT_PRECIS,       // nOutPrecision
                                  CLIP_DEFAULT_PRECIS,      // nClipPrecision
                                  DEFAULT_QUALITY,          // nQuality
                                  DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily
                                  m_csFontStyle));
}
void CCustomClock::DestroyClock() {
    m_cfont.DeleteObject();
    m_cFontAMPM.DeleteObject();
    m_cFontDate.DeleteObject();
}
CString CCustomClock::GetDateTime() {
    SYSTEMTIME sysTime;
    WORD       wHour = 0;
    CString    csDateTime;

    GetLocalTime(&sysTime);
    wHour = sysTime.wHour;
    if (sysTime.wHour == 0)
        wHour = 12;
    else if (sysTime.wHour > 12)
        wHour = sysTime.wHour - 12;

    csDateTime.Format(_T("%s %02d, %04d "), pCCustomClock->GetMonthName(sysTime.wMonth).GetBuffer(),
                      sysTime.wDay, sysTime.wYear);

    if (sysTime.wHour >= 12)
        csDateTime.AppendFormat(_T("%d:%02d:%02d PM"), wHour, sysTime.wMinute, sysTime.wSecond);
    else
        csDateTime.AppendFormat(_T("%d:%02d:%02d AM"), wHour, sysTime.wMinute, sysTime.wSecond);

    return csDateTime;
}
void CCustomClock::DrawClock(CClientDC* dcSrc, int x, int y) {
    // 1. Get DPI scaling
    int    dpiX   = GetDeviceCaps(dcSrc->GetSafeHdc(), LOGPIXELSX);
    int    dpiY   = GetDeviceCaps(dcSrc->GetSafeHdc(), LOGPIXELSY);
    double scaleX = dpiX / 96.0;
    double scaleY = dpiY / 96.0;

    int xScaled = static_cast<int>(x * scaleX);
    int yScaled = static_cast<int>(y * scaleY);

    // 2. Clear previous clock area with inflation for padding
    CRect inflatedRect = m_rectClock;
    inflatedRect.InflateRect((int)(scaleX), (int)(scaleY));
    dcSrc->FillSolidRect(&inflatedRect, m_textBKColor);

    // 3. Prepare variables
    SYSTEMTIME sysTime;
    SIZE       sizeTime = {}, sizeAMPM = {}, sizeDate = {};
    TEXTMETRIC tmTime = {}, tmDate = {};
    CString    csTime, csAMPM, csDate;
    WORD       wHour = 0;

    // 4. Get current time
    GetLocalTime(&sysTime);
    wHour = sysTime.wHour;
    if (wHour == 0)
        wHour = 12;
    else if (wHour > 12)
        wHour -= 12;

    // 5. Format strings
    csTime.Format(_T("%d:%02d:%02d"), wHour, sysTime.wMinute, sysTime.wSecond);
    csAMPM = (sysTime.wHour >= 12) ? _T(" PM") : _T(" AM");
    csDate.Format(_T("%s, %s %02d, %04d"),
                  CalcDayOfWeek(sysTime.wYear, sysTime.wMonth, sysTime.wDay).GetBuffer(),
                  GetMonthName(sysTime.wMonth).GetBuffer(), sysTime.wDay, sysTime.wYear);

    // 6. Draw time
    dcSrc->SetBkMode(TRANSPARENT);
    dcSrc->SetTextColor(m_textColor);
    CFont* pOldFont = dcSrc->SelectObject(&m_cfont);
    dcSrc->TextOut(xScaled, yScaled, csTime);
    GetTextExtentPoint32(dcSrc->GetSafeHdc(), csTime, csTime.GetLength(), &sizeTime);
    GetTextMetrics(dcSrc->GetSafeHdc(), &tmTime);
    dcSrc->SelectObject(pOldFont);

    // 7. Draw AM/PM
    dcSrc->SetTextColor(m_textColor);
    pOldFont = dcSrc->SelectObject(&m_cFontAMPM);
    dcSrc->TextOut(xScaled + sizeTime.cx, yScaled + (int)(tmTime.tmAscent * 0.5), csAMPM);
    GetTextExtentPoint32(dcSrc->GetSafeHdc(), csAMPM, csAMPM.GetLength(), &sizeAMPM);
    dcSrc->SelectObject(pOldFont);

    // 8. Draw Date
    dcSrc->SetTextColor(m_textColor);
    pOldFont  = dcSrc->SelectObject(&m_cFontDate);
    int yDate = yScaled + tmTime.tmHeight;
    dcSrc->TextOut(xScaled, yDate, csDate);
    GetTextExtentPoint32(dcSrc->GetSafeHdc(), csDate, csDate.GetLength(), &sizeDate);
    GetTextMetrics(dcSrc->GetSafeHdc(), &tmDate);
    dcSrc->SelectObject(pOldFont);

    // 9. Update clock rect for the next redraw
    int totalWidth  = max(sizeTime.cx + sizeAMPM.cx, sizeDate.cx);
    int totalHeight = tmTime.tmHeight + tmDate.tmHeight;

    m_rectClock.left   = xScaled;
    m_rectClock.top    = yScaled;
    m_rectClock.right  = xScaled + totalWidth;
    m_rectClock.bottom = yScaled + totalHeight;
}

ULONG CCustomClock::CalcDayNumFromDate(UINT y, UINT m, UINT d) {
    m = (m + 9) % 12;
    y -= m / 10;
    ULONG dn = 365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + (d - 1);

    return dn;
}

// ----------------------------------------------------------------------
// Given year, month, day, return the day of week (string).
// ----------------------------------------------------------------------
CString CCustomClock::CalcDayOfWeek(WORD y, WORD m, WORD d) {
    CString day[] = {_T("Wednesday"), _T("Thursday"), _T("Friday"), _T("Saturday"),
                     _T("Sunday"),    _T("Monday"),   _T("Tuesday")};

    ULONG dn = CalcDayNumFromDate(y, m, d);

    return day[dn % 7];
}

CString CCustomClock::GetMonthName(WORD wMonth) {
    CString day[] = {_T("Error"),   _T("January"),  _T("February"), _T("March"),  _T("April"),
                     _T("May"),     _T("June"),     _T("July"),     _T("August"), _T("September"),
                     _T("October"), _T("November"), _T("December")};

    return day[wMonth];
}
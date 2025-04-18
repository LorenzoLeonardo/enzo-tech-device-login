#pragma once
class CScalableBitmapStatic : public CStatic
{
public:
    void SetBitmapResource(UINT nResID);

protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()

private:
    UINT m_bitmapResID = 0;
};

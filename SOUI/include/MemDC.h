#pragma once

namespace SOUI
{

class CMemDC :	public CDC
{
public:
    CMemDC();
    CMemDC(HDC hdc, const CRect & rc);
    CMemDC(HDC hdc, HBITMAP hBmp);
    ~CMemDC(void);

    HBITMAP SelectBitmap(HBITMAP hBmp);

    BOOL	HasBitmap()
    {
        return m_bHasBitmap;
    }

	void SetBitmapOwner(BOOL bOwner)
	{
		m_bBmpOwner=bOwner;
	}
	BOOL IsBitmapOwner(){return m_bBmpOwner;}

	void DeleteDC();

	BOOL InitDC(HDC hdc,const CRect &rc);

protected:
    HBITMAP		m_hOldBmp;
    BOOL		m_bBmpOwner;
    BOOL		m_bHasBitmap;
};

}//namespace SOUI
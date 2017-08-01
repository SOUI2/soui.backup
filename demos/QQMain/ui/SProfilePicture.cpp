#include "stdafx.h"
#include "SProfilePicture.h"
#include <helper/SDIBHelper.h>


namespace SOUI
{
    SProfilePicture::SProfilePicture()
        : m_bDraging(FALSE)
        , m_eMoveType(TYPE_UNKNOW)
    {
        m_evtSet.addEvent(EVENTID(SProfilePicture::EventSelFrameChange));
    }


    SProfilePicture::~SProfilePicture()
    {
    }

    void SProfilePicture::SetHeadPic(const SStringT& szPath)
    {
        m_bmpHead = SResLoadFromFile::LoadImage(szPath);
        InitHeadPic();
        Invalidate();
    }

    void SProfilePicture::InitHeadPic()
    {
        CRect rcClient = GetClientRect();
        int nHeight = m_bmpHead->Height();
        int nWidth = m_bmpHead->Width();
        CRect rcBmp(0, 0, nWidth, nHeight);

        float scaleX = (float)rcClient.Width() / nWidth;
        float scaleY = (float)rcClient.Height() / nHeight;

        if ((scaleX < 1) || (scaleY < 1))
        {
            if (scaleX < scaleY)
            {
                nWidth = nWidth * scaleX;
                nHeight = nHeight * scaleX;
            }
            else
            {
                nWidth = nWidth * scaleY;
                nHeight = nHeight * scaleY;
            }
        }

        m_nY = (rcClient.Height() - nHeight + 1) / 2;
        m_nX = (rcClient.Width() - nWidth + 1) / 2;

        int nMoveWH = nWidth > nHeight ? nHeight : nWidth;
        m_BeginPoint.SetPoint(rcClient.left + m_nX, rcClient.top + m_nY);
        m_EndPoint.SetPoint(m_BeginPoint.x + nMoveWH, m_BeginPoint.y + nMoveWH);

        m_PicRect.SetRect(rcClient.left + m_nX, rcClient.top + m_nY, rcClient.left + m_nX + nWidth, rcClient.top + m_nY + nHeight);
        m_FrameRect.SetRect(m_BeginPoint, m_EndPoint);
        m_FrameRect.OffsetRect(-m_PicRect.left, -m_PicRect.top);

        CAutoRefPtr<IRenderTarget> pTempRT;
        GETRENDERFACTORY->CreateRenderTarget(&pTempRT, nWidth, nHeight);
        pTempRT->ClearRect(CRect(0, 0, nWidth, nHeight), RGBA(248, 248, 248, 255));
        pTempRT->DrawBitmapEx(CRect(0, 0, nWidth, nHeight), m_bmpHead, rcBmp, MAKELONG(EM_STRETCH, kHigh_FilterLevel));
        m_bmpHead->Release();

        if (!m_pScreenRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_pScreenRT, nWidth, nHeight);
        }
        else
        {
            m_pScreenRT->Resize(CSize(nWidth, nHeight));
        }

        if (!m_pHuiRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_pHuiRT, nWidth, nHeight);
        }
        else
        {
            m_pHuiRT->Resize(CSize(nWidth, nHeight));
        }

        if (!m_pBufferRT)
        {
            GETRENDERFACTORY->CreateRenderTarget(&m_pBufferRT, nMoveWH, nMoveWH);
        }
        else
        {
            m_pBufferRT->Resize(CSize(nMoveWH, nMoveWH));
        }

        m_pScreenRT->BitBlt(CRect(0, 0, nWidth, nHeight), pTempRT, 0, 0);
        CAutoRefPtr<IBitmap> huiBmp;
        GETRENDERFACTORY->CreateBitmap(&huiBmp);
        huiBmp->Init(nWidth, nHeight);
        m_pHuiRT->SelectObject(huiBmp);
        m_pHuiRT->BitBlt(CRect(0, 0, nWidth, nHeight), m_pScreenRT, 0, 0);
        SDIBHelper::GrayImage(huiBmp);

        if (!m_bmpSelHead)
            GETRENDERFACTORY->CreateBitmap(&m_bmpSelHead);
        m_bmpSelHead->Init(nMoveWH, nMoveWH);
        m_pBufferRT->SelectObject(m_bmpSelHead);
        m_pBufferRT->BitBlt(CRect(0, 0, nMoveWH, nMoveWH), m_pScreenRT, m_FrameRect.left, m_FrameRect.top);

        EventSelFrameChange evt(this);
        evt.iBmp = m_bmpSelHead;
        FireEvent(evt);
    }

    void SProfilePicture::OnLButtonDown(UINT nFlags, CPoint pt)
    {
        CRect rc(m_BeginPoint, m_EndPoint);
        CRect rcLeftTop(m_BeginPoint.x - 3, m_BeginPoint.y - 3, m_BeginPoint.x + 3, m_BeginPoint.y + 3);
        CRect rcLeftBottom(m_BeginPoint.x - 3, m_EndPoint.y - 3, m_BeginPoint.x + 3, m_EndPoint.y + 3);
        CRect rcRightTop(m_EndPoint.x - 3, m_BeginPoint.y - 3, m_EndPoint.x + 3, m_BeginPoint.y + 3);
        CRect rcRightBottom(m_EndPoint.x - 3, m_EndPoint.y - 3, m_EndPoint.x + 3, m_EndPoint.y + 3);
        if (rc.PtInRect(pt) || rcLeftTop.PtInRect(pt) || rcLeftBottom.PtInRect(pt) || rcRightTop.PtInRect(pt) || rcRightBottom.PtInRect(pt))
        {
            SetCapture();
            m_bDraging = TRUE;
            m_ptClick = pt;
            if (rc.PtInRect(pt))
            {
                m_eMoveType = TYPE_ALL;
                ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizeall")));
            }
            if (rcLeftTop.PtInRect(pt))
            {
                m_eMoveType = TYPE_LEFTTOP;
                ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizenwse")));
            }
            else if (rcLeftBottom.PtInRect(pt))
            {
                m_eMoveType = TYPE_LEFTBOTTOM;
                ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizenesw")));
            }
            else if (rcRightTop.PtInRect(pt))
            {
                m_eMoveType = TYPE_RIGHTTOP;
                ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizenesw")));
            }
            else if (rcRightBottom.PtInRect(pt))
            {
                m_eMoveType = TYPE_RIGHTBOTTOM;
                ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizenwse")));
            }
        }
    }

    void SProfilePicture::OnLButtonUp(UINT nFlags, CPoint pt)
    {
        m_bDraging = FALSE;
        ReleaseCapture();
    }

    void SProfilePicture::OnMouseMove(UINT nFlags, CPoint pt)
    {
        SetMsgHandled(FALSE);
        CRect rc(m_BeginPoint, m_EndPoint);
        CRect rcLeftTop(m_BeginPoint.x - 3, m_BeginPoint.y - 3, m_BeginPoint.x + 3, m_BeginPoint.y + 3);
        CRect rcLeftBottom(m_BeginPoint.x - 3, m_EndPoint.y - 3, m_BeginPoint.x + 3, m_EndPoint.y + 3);
        CRect rcRightTop(m_EndPoint.x - 3, m_BeginPoint.y - 3, m_EndPoint.x + 3, m_BeginPoint.y + 3);
        CRect rcRightBottom(m_EndPoint.x - 3, m_EndPoint.y - 3, m_EndPoint.x + 3, m_EndPoint.y + 3);
        if (rc.PtInRect(pt))
        {
            ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizeall")));
        }
        if (rcLeftTop.PtInRect(pt) || rcRightBottom.PtInRect(pt))
        {
            ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizenwse")));
        }
        if (rcLeftBottom.PtInRect(pt) || rcRightTop.PtInRect(pt))
        {
            ::SetCursor(GETRESPROVIDER->LoadCursor(_T("sizenesw")));
        }
        if (m_bDraging)
        {
            CRect rcClient = GetClientRect();
            int nWH = m_EndPoint.x - m_BeginPoint.x;
            CPoint  ptLT = pt - m_ptClick;
            if (m_eMoveType == TYPE_ALL)
            {
                m_BeginPoint.Offset(ptLT);
                m_EndPoint.Offset(ptLT);
            }
            else if (m_eMoveType == TYPE_LEFTTOP)
            {
                if (ptLT.y > ptLT.x)
                    ptLT.x = ptLT.y;
                else
                    ptLT.y = ptLT.x;
                m_BeginPoint.Offset(ptLT);
            }
            else if (m_eMoveType == TYPE_LEFTBOTTOM)
            {
                if (ptLT.y > -ptLT.x)
                    ptLT.y = -ptLT.x;
                else
                    ptLT.x = -ptLT.y;
                m_BeginPoint.Offset(ptLT.x, 0);
                m_EndPoint.Offset(0, ptLT.y);
            }
            else if (m_eMoveType == TYPE_RIGHTTOP)
            {
                if (ptLT.y > -ptLT.x)
                    ptLT.y = -ptLT.x;
                else
                    ptLT.x = -ptLT.y;
                m_BeginPoint.Offset(0, ptLT.y);
                m_EndPoint.Offset(ptLT.x, 0);
            }
            else if (m_eMoveType == TYPE_RIGHTBOTTOM)
            {
                if (ptLT.y > ptLT.x)
                    ptLT.x = ptLT.y;
                else
                    ptLT.y = ptLT.x;
                m_EndPoint.Offset(ptLT);
            }
            pt = m_ptClick + ptLT;
            if (m_BeginPoint.x < m_nX + rcClient.left)
            {
                m_BeginPoint.x = m_nX + rcClient.left;
                m_EndPoint.x = m_BeginPoint.x + nWH;
            }
            if (m_EndPoint.x > rcClient.right - m_nX)
            {
                m_EndPoint.x = rcClient.right - m_nX;
                m_BeginPoint.x = m_EndPoint.x - nWH;
            }
            if (m_BeginPoint.y < m_nY + rcClient.top)
            {
                m_BeginPoint.y = m_nY + rcClient.top;
                m_EndPoint.y = m_BeginPoint.y + nWH;
            }
            if (m_EndPoint.y > rcClient.bottom - m_nY)
            {
                m_EndPoint.y = rcClient.bottom - m_nY;
                m_BeginPoint.y = m_EndPoint.y - nWH;
            }
            m_ptClick = pt;

            m_bmpSelHead->Init(m_EndPoint.x - m_BeginPoint.x, m_EndPoint.y - m_BeginPoint.y);
            m_FrameRect.SetRect(m_BeginPoint, m_EndPoint);
            m_FrameRect.OffsetRect(-m_PicRect.left, -m_PicRect.top);
            m_pBufferRT->SelectObject(m_bmpSelHead);
            m_pBufferRT->BitBlt(CRect(0, 0, m_FrameRect.Width(), m_FrameRect.Height()), m_pScreenRT, m_FrameRect.left, m_FrameRect.top);

            EventSelFrameChange evt(this);
            evt.iBmp = m_bmpSelHead;
            FireEvent(evt);

            Invalidate();
        }
    }

    void SProfilePicture::OnPaint(IRenderTarget *pRT)
    {
        if (m_bmpHead == NULL)
        {
            SWindow::OnPaint(pRT);
        }
        else
        {
            CRect rcClient = GetClientRect();
            pRT->FillSolidRect(rcClient, RGBA(248, 248, 248, 255));

            pRT->BitBlt(m_PicRect, m_pHuiRT, 0, 0);
            CRect temp(m_BeginPoint, m_EndPoint);
            pRT->BitBlt(temp, m_pBufferRT, 0, 0);

            CPoint point[4] =
            {
                CPoint(m_BeginPoint.x, m_BeginPoint.y),	//左上
                CPoint(m_EndPoint.x, m_BeginPoint.y),	//右上
                CPoint(m_BeginPoint.x, m_EndPoint.y),   //左下
                CPoint(m_EndPoint.x, m_EndPoint.y ),		//右下
            };
            CPoint line1[2] = { point[0], point[1] };
            CPoint line2[2] = { point[0], point[2] };
            CPoint line3[2] = { point[1], point[3] };
            CPoint line4[2] = { point[2], point[3] };

            CAutoRefPtr<IPen> pen, oldpen;
            pRT->CreatePen(PS_DASH, RGBA(0, 0, 0, 255), 1, &pen);
            pRT->SelectObject(pen, (IRenderObj**)&oldpen);
            pRT->DrawLines(line1, 2);
            pRT->DrawLines(line2, 2);
            pRT->DrawLines(line3, 2);
            pRT->DrawLines(line4, 2);
            pRT->SelectObject(oldpen);

            for (int i = 0; i < 4; ++i)
            {
                pRT->DrawRectangle(CRect(point[i].x - 3, point[i].y - 3, point[i].x + 3, point[i].y + 3));
                CAutoRefPtr<IBrush> brush, oldBrush;
                pRT->CreateSolidColorBrush(RGBA(255, 255, 255, 255), &brush);
                pRT->SelectObject(brush, (IRenderObj**)&oldBrush);
                pRT->FillRectangle(CRect(point[i].x - 3, point[i].y - 3, point[i].x + 2, point[i].y + 2));
                //pRT->FillRectangle(CRect(point[i].x - 1, point[i].y - 1, point[i].x + 1, point[i].y + 1));
                pRT->SelectObject(oldBrush);
            }
        }
    }
}
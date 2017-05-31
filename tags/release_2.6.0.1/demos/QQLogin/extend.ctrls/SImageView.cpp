#include "stdafx.h"
#include "souistd.h"
#include "SImageView.h"
#include "utils.h"
#include "extend.skins\SAntialiasSkin.h"

namespace SOUI
{

    SImageView::SImageView() :_pImageSkin(NULL),
        _currentFrame(0),
        _nextFrameInterval(0),
        _isPlaying(TRUE)
    {
        _imageSize.cx = _imageSize.cy = 0; // 默认显示的图片大小
    }

    SImageView::~SImageView()
    {
        if (_pImageSkin)
        {
            _pImageSkin->Release();
            _pImageSkin = NULL;
        }
        GetContainer()->UnregisterTimelineHandler(this);
    }

    void SImageView::OnPaint(IRenderTarget *pRT)
    {
        SWindow::OnPaint(pRT);

        if (_pImageSkin)
        {
            _pImageSkin->Draw(pRT, GetWindowRect(), _currentFrame);

            if (_isPlaying && _pImageSkin->GetStates() > 1)
            {
                GetContainer()->RegisterTimelineHandler(this);
            }
        }
    }

    void SImageView::OnNextFrame()
    {
        _nextFrameInterval -= 10;
        if (_nextFrameInterval <= 0 && _pImageSkin)
        {
            GetContainer()->UnregisterTimelineHandler(this);

            int nStates = _pImageSkin->GetStates();
            if (nStates <= 1)
            {
                return;
            }

            _currentFrame += 1;
            _currentFrame %= nStates;

            _nextFrameInterval = GetFrameDelay();
            if (_nextFrameInterval < 80)
            {
                _nextFrameInterval = 80;
            }

            Invalidate();
        }
    }

    void SImageView::OnShowWindow(BOOL bShow, UINT nStatus)
    {
        SWindow::OnShowWindow(bShow, nStatus);

        if (!bShow)
        {
            GetContainer()->UnregisterTimelineHandler(this);
        }
        else if (_pImageSkin && _isPlaying && _pImageSkin->GetStates() > 1)
        {
            GetContainer()->RegisterTimelineHandler(this);
            if (GetFrameDelay() == 0)
            {
                _nextFrameInterval = 90;
            }
            else
            {
                _nextFrameInterval = GetFrameDelay();
            }
        }
    }

    HRESULT SImageView::OnAttrSkin(const SStringW & strValue, BOOL bLoading)
    {
        ISkinObj *pSkin = SSkinPoolMgr::getSingleton().GetSkin(strValue, GetScale());
        if (!pSkin || !pSkin->IsClass(SAntialiasSkin::GetClassName()))
        {
            return S_FALSE;
        }
        SetSkin(pSkin);

        return bLoading ? S_OK : S_FALSE;
    }

    CSize SImageView::GetDesiredSize(LPCRECT /*pRcContainer*/)
    {
        CSize sz;
        if (_pImageSkin)
        {
            sz = _pImageSkin->GetSkinSize();
        }

        return sz;
    }

    BOOL SImageView::IsPlaying()
    {
        return _isPlaying;
    }

    int  SImageView::GetFrameCount()
    {
        if (_pImageSkin)
        {
            return _pImageSkin->GetStates();
        }
        return 0;
    }

    SIZE SImageView::GetImageSize()
    {
        return _imageSize;
    }

    void SImageView::SetImageSize(SIZE size)
    {
        _imageSize = size;
    }

    SStringW SImageView::GetRealPath()
    {
        return _realPath;
    }

    void SImageView::SetSkin(ISkinObj* pSkin)
    {
        if (pSkin)
        {
            pSkin->AddRef();
        }

        if (_pImageSkin)
        {
            _pImageSkin->Release();
        }

        _pImageSkin = pSkin;

        Invalidate();
    }

    ISkinObj* SImageView::GetSkin()
    {
        return _pImageSkin;
    }

    int  SImageView::GetFrameDelay()
    {
        if (_pImageSkin && _pImageSkin->IsClass(SAntialiasSkin::GetClassName()))
        {
            return ((SAntialiasSkin*)_pImageSkin)->GetFrameDelay() * 10;
        }

        return 0;
    }

    void SImageView::Pause()
    {
        if (_pImageSkin && _pImageSkin->GetStates() > 1)
        {
            GetContainer()->UnregisterTimelineHandler(this);
        }
        _isPlaying = FALSE;
    }

    void SImageView::Resume()
    {
        if (_pImageSkin && _pImageSkin->GetStates() > 1)
        {
            GetContainer()->RegisterTimelineHandler(this);
        }

        _isPlaying = TRUE;
    }

    void SImageView::ShowFrame(int frame, BOOL update/*=FALSE*/)
    {
        if (frame < 0 || frame >= GetFrameCount())
        {
            return;
        }
        _currentFrame = frame;

        if (update)
        {
            Invalidate();
        }
    }

    void SImageView::OnDestroy()
    {
        GetContainer()->UnregisterTimelineHandler(this);
        __super::OnDestroy();
    }
} // SOUI

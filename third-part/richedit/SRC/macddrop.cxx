//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  File:       MacDDrop.cxx
//
//  Contents:   Our own implementation of RegisterDragDrop and DoDragDrop
//
//----------------------------------------------------------------------------
#ifdef MACPORT
#include "_common.h"
#include "stdio.h"

#include "sys\timeb.h"

ASSERTDATA

#ifdef PeekMessage
#undef PeekMessage
#endif
#define PeekMessage PeekMessageA

//+------------------------------------------------------------------------
//
//  Class:      DropTargetPair
//
//  Notes:      Implements a simple dlink list to hold 
//              DragDrop hWnd/DropTarget pairs
//
//-------------------------------------------------------------------------
class DropTargetPair
{
public:
    HWND                    _hwnd; 
    LPDROPTARGET            _pDropTarget;
    DropTargetPair *        _pPrev;
    DropTargetPair *        _pNext;


    DropTargetPair(HWND hwnd, LPDROPTARGET pDropTarget);
    void RemoveDropTargetPair (HWND hwnd);
    DropTargetPair *FindhWnd (HWND hwnd);
    DropTargetPair *FindhWnd (POINT screenPt);
} ;

typedef DropTargetPair *DropTargetPairPtr;


static DropTargetPairPtr g_pDropTargetPairList = NULL;

DropTargetPair::DropTargetPair(HWND hwnd, LPDROPTARGET pDropTarget)
{
    _hwnd = hwnd; 
    _pDropTarget = pDropTarget; 
    _pDropTarget->AddRef();
    _pPrev = NULL; 
    _pNext = g_pDropTargetPairList;
    g_pDropTargetPairList->_pPrev = this;
    g_pDropTargetPairList = this;
}

DropTargetPair *DropTargetPair::FindhWnd (HWND hwnd)
{
    HWND    hWndParent;

    DropTargetPairPtr pDTP = g_pDropTargetPairList;
    while (pDTP)
    {
        if (pDTP->_hwnd == hwnd)
            return pDTP;
        pDTP = pDTP->_pNext;
    }
    hWndParent = GetParent(hwnd);
    if (hWndParent)
        return this->FindhWnd( hWndParent);
    return NULL;
}
DropTargetPair *DropTargetPair::FindhWnd (POINT screenPt)
{
    RECT    rect;
    DropTargetPairPtr pDTP = g_pDropTargetPairList;
    while (pDTP)
    {
        GetWindowRect (pDTP->_hwnd, &rect);
        if (PtInRect(&rect,screenPt))
            return pDTP;
        pDTP = pDTP->_pNext;
    }
    return NULL;
}
void DropTargetPair::RemoveDropTargetPair (HWND hwnd)
{
    DropTargetPairPtr pDTP = FindhWnd(hwnd);
    if(pDTP)
    {
        if(pDTP->_pPrev)
            pDTP->_pPrev->_pNext = pDTP->_pNext;
        if(pDTP->_pNext)
            pDTP->_pNext->_pPrev = pDTP->_pPrev;

        if( pDTP == g_pDropTargetPairList)
            g_pDropTargetPairList = pDTP->_pNext;
 
        pDTP->_pDropTarget->Release();
        delete pDTP;
    }
}


STDAPI  MacRegisterDragDrop (    HWND hwnd, 
                                    LPDROPTARGET pDropTarget)
{
    DropTargetPairPtr pDTP = new DropTargetPair(hwnd,pDropTarget);

    return S_OK;
}
STDAPI  MacRevokeDragDrop (HWND hwnd)
{
    g_pDropTargetPairList->RemoveDropTargetPair(hwnd);


    return S_OK;
}
static void  SetDragDropCursor ( DWORD         dwEffect)
{
    static HCURSOR hcursorArrow = NULL;
    static HCURSOR hcursorDragNone;
    static HCURSOR hcursorDragCopy;
    static HCURSOR hcursorDragMove;
#ifdef MACPORT
    if ( NULL == hcursorArrow)
    {
        hcursorArrow     = LoadCursor ( NULL, IDC_ARROW );
        hcursorDragNone  = LoadCursor ( hinstRE, MAKEINTRESOURCE(IDC_DRAGDROPNONE) );
        hcursorDragCopy  = LoadCursor ( hinstRE, MAKEINTRESOURCE(IDC_DRAGDROPCOPY) );
        hcursorDragMove  = LoadCursor ( hinstRE, MAKEINTRESOURCE(IDC_DRAGDROPMOVE) );
    }
#endif

    switch(dwEffect)
    {
    case DROPEFFECT_COPY:
        SetCursor(hcursorDragCopy);
        break;
    case DROPEFFECT_MOVE:
        SetCursor(hcursorDragMove);
        break;
    case DROPEFFECT_NONE:
        SetCursor(hcursorDragNone);
        break;
    default:
        SetCursor(hcursorArrow);
        break;

    }


}
STDAPI  MacDoDragDrop (  LPDATAOBJECT    pDataObj,
                            LPDROPSOURCE    pDropSource,
                            DWORD           dwOKEffects,
                            LPDWORD         pdwEffect)
{
    HRESULT             hr;
    DropTargetPairPtr   pDTP;
    DropTargetPairPtr   pNewDTP;
    POINTL              clientPtl;
    POINTL              screenPtl;
    POINT               screenPt;
    BOOL                fEscape;
    DWORD               dwEff;
    BOOL                fOther;
    HWND                hWnd;
    BOOL                fCapture = FALSE;
    MSG                 gMsg;
    BOOL                gfTracking;
    BOOL                bHaveMsg;

    bHaveMsg = PeekMessage(&gMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE);

    hWnd = GetCapture();
    if (NULL == hWnd)
    {
        if (!bHaveMsg)
        {
            // don't know which hwnd I am, need to wait for a mouse move
            while(!bHaveMsg)
            {
                bHaveMsg = PeekMessage(&gMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE);
            }
        }
        SetCapture (gMsg.hwnd);
        fCapture = TRUE;
        hWnd = gMsg.hwnd;
    }

    pDTP = g_pDropTargetPairList->FindhWnd(hWnd);
    if (NULL == pDTP)
    {
        // hWnd is not a registered DropTarget
        hr = DRAGDROP_S_CANCEL;
        goto Cleanup;
    }

    *pdwEffect = dwOKEffects;   // Initialize allowed effects

    clientPtl.x = LOWORD(gMsg.lParam);
    clientPtl.y = HIWORD(gMsg.lParam);
    dwEff = *pdwEffect;
    hr = pDTP->_pDropTarget->DragEnter( pDataObj, gMsg.wParam, clientPtl, &dwEff);
    if (hr)
    {
        hr = DRAGDROP_S_CANCEL;
        goto Cleanup;
    }
 
    hr = pDropSource->GiveFeedback ( dwEff );
    if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
    {
        SetDragDropCursor (dwEff);
    }

    //
    //  Modal message loop.
    //
    fEscape = FALSE;
    gfTracking = TRUE;
    while (gfTracking)
    {
        fOther = FALSE;
        if (PeekMessage(&gMsg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
            PeekMessage(&gMsg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE) ||
            PeekMessage(&gMsg, NULL, 0, 0, PM_REMOVE))
        {

        // Dispatch other messages that are not mouse or keyboard

            if (!((gMsg.message >= WM_MOUSEFIRST) && (gMsg.message <= WM_MOUSELAST) ||
                  (gMsg.message >= WM_KEYFIRST)   && (gMsg.message <= WM_KEYLAST)))
            {
                DispatchMessage(&gMsg);
                continue;
            }

            screenPt.x = (short)(LOWORD(gMsg.lParam));
            screenPt.y = (short)(HIWORD(gMsg.lParam));
            ClientToScreen(gMsg.hwnd,&screenPt);
            screenPtl.x = screenPt.x;
            screenPtl.y = screenPt.y;
            pNewDTP = g_pDropTargetPairList->FindhWnd(screenPt);

        // We need a failsafe here in case the LBUTTONUP got lost

            if( gMsg.message == WM_MOUSEMOVE && !(gMsg.wParam & MK_LBUTTON))
                gMsg.message = WM_LBUTTONUP;

            switch(gMsg.message)
            {
            case  WM_MOUSEMOVE:
                // has mouse has left the current window?
                Assert(gMsg.wParam & MK_LBUTTON);  // mouse button must be down!
                if( pDTP != pNewDTP)
                {
                    if (pDTP)
                    {
                        pDTP->_pDropTarget->DragLeave ( );
                        hr = pDropSource->GiveFeedback ( DROPEFFECT_NONE );
                        if (DRAGDROP_S_USEDEFAULTCURSORS == hr && NULL == pNewDTP)
                        {
                            SetDragDropCursor (DROPEFFECT_NONE);
                        }
                        pDTP = NULL;
                    }

                    // has the mouse entered another registered window?
                    if (pNewDTP)
                    {   
                        pDTP = pNewDTP;
                        dwEff = *pdwEffect;
                        pDTP->_pDropTarget->DragEnter ( pDataObj,
                                                        gMsg.wParam,  
                                                        screenPtl,
                                                        &dwEff);

                        hr = pDropSource->GiveFeedback ( dwEff );
                        if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
                        {
                            SetDragDropCursor (dwEff);
                        }
                    }
                    break;
                }

                if (pDTP)
                {
                    // mouse must have moved within the same window
                    dwEff = *pdwEffect;
                    pDTP->_pDropTarget->DragOver ( gMsg.wParam,
                                                   screenPtl,
                                                   &dwEff);
                    hr = pDropSource->GiveFeedback ( dwEff );
                    if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
                    {
                        SetDragDropCursor (dwEff);
                    }
                }
                break;
            case WM_CHAR:
                fEscape = (0x1B == gMsg.wParam);
                break;
            case WM_LBUTTONUP:
                if (pNewDTP == NULL)
                {   // a button up in a non-registered window means cancel.
                    hr = DRAGDROP_S_CANCEL;
                    goto Cleanup;
                }
              // fall thru
            default:
                // must be keyboard or mouse button state change
                if (pDTP != pNewDTP)
                {
                    // mouse has move into a new window
                    if (pDTP)
                    {
                        pDTP->_pDropTarget->DragLeave ( );
                    }
                    pDTP = pNewDTP;
                    if (pDTP)
                    {
                        dwEff = *pdwEffect;
                        pDTP->_pDropTarget->DragEnter ( pDataObj,
                                                        gMsg.wParam,  
                                                        screenPtl,
                                                        &dwEff);

                        hr = pDropSource->GiveFeedback ( dwEff );
                        if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
                        {
                            SetDragDropCursor (dwEff);
                        }
                    }
                }
                hr = pDropSource->QueryContinueDrag ( fEscape,gMsg.wParam );
                fEscape = FALSE;
                switch( hr)
                {
                    case S_OK:
                        if (pDTP)
                        {
                            dwEff = *pdwEffect;
                            pDTP->_pDropTarget->DragOver( gMsg.wParam,
                                                          screenPtl,
                                                          &dwEff);

                            hr = pDropSource->GiveFeedback ( dwEff );
                            if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
                            {
                                SetDragDropCursor (dwEff);
                            }
                        }
                        break;
                    case DRAGDROP_S_DROP:
                        if (pDTP)
                        {
                            dwEff = *pdwEffect;
                            pDTP->_pDropTarget->Drop ( pDataObj,
                                                       gMsg.wParam,
                                                       screenPtl,
                                                       &dwEff);
                         //   hr = pDropSource->GiveFeedback ( dwEff );
                         //   if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
                         //   {
                         //       SetDragDropCursor (dwEff);
                         //       hr = DRAGDROP_S_DROP;
                         //   }
                        }
                        else
                        {
                            hr = DRAGDROP_S_CANCEL;
                        }
                        gfTracking = FALSE;
                        break;
                    case DRAGDROP_S_CANCEL:
                        if (pDTP)
                        {
                            pDTP->_pDropTarget->DragLeave ( );
                            hr = pDropSource->GiveFeedback ( DROPEFFECT_NONE );
                            if (DRAGDROP_S_USEDEFAULTCURSORS == hr)
                            {
                                SetDragDropCursor (dwEff);
                            }
                        }
                        gfTracking = FALSE;
                        break;
                }
                break;
            }
        }

    }

    *pdwEffect = dwEff;
Cleanup:
    if (fCapture)
        ReleaseCapture();

OutputDebugStringA("Leaving MacDoDragDrop");
    return hr;
}

#endif // MACPORT
#include "stdafx.h"
#include "SChatEdit.h"
#include "reole/RichEditOle.h"

const GUID __declspec(selectany) CLSID_CSoSmileyCtrl =
{0xd29e0bde,0xcfda,0x4b93,{0x92,0x9a,0x87,0x7a,0xb4,0x55,0x7b,0xd8}};

namespace SOUI{

    const SStringW KLabelColor      = L"color";
    const SStringW KLabelLink       = L"link";
    const SStringW KLabelFont       = L"font";
    const SStringW KLabelUnderline  = L"underline";
    const SStringW KLabelItalic     = L"italic";
    const SStringW KLabelBold       = L"bold";
    const SStringW KLabelStrike     = L"strike";
    const SStringW KLabelSmiley     = L"smiley";

    SChatEdit::SChatEdit(void)
    {
        GetEventSet()->addEvent(EventChatEditKeyReturn::EventID);
    }

    SChatEdit::~SChatEdit(void)
    {
    }    

    BOOL SChatEdit::AppendFormatText(const SStringW & strMsg)
    {
        pugi::xml_document doc;
        if(!doc.load_buffer((LPCWSTR)strMsg,strMsg.GetLength()*2,pugi::parse_default,pugi::encoding_utf16)) return FALSE;
        return AppendFormatText(doc);
    }

    BOOL SChatEdit::AppendFormatText(const pugi::xml_node xmlMsg)
    {
        TCHAR szRet[]={0x0a,0};
        int nLen = (int)SSendMessage(WM_GETTEXTLENGTH);
        SSendMessage(EM_SETSEL,nLen,nLen);
        SSendMessage(EM_REPLACESEL,FALSE,(LPARAM)L"\r\n");
        nLen = (int)SSendMessage(WM_GETTEXTLENGTH);
        SSendMessage(EM_SETSEL,nLen,nLen);
        long iCaret = nLen;

        CHARFORMAT cf={0};
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_ALL;
        SSendMessage(EM_GETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);        
        cf.dwEffects &= ~CFE_AUTOCOLOR;
        _AppendFormatText(iCaret,cf,xmlMsg);
        
        SSendMessage(WM_VSCROLL,MAKEWPARAM(SB_BOTTOM,0));
        return TRUE;
    }

    int SChatEdit::_AppendFormatText(int iCaret,CHARFORMAT cf,pugi::xml_node xmlText)
    {
        SStringW strText = xmlText.value();
        if(xmlText.name() == KLabelSmiley)
        {//insert smiley
            SComPtr<ISoSmileyCtrl> pSmiley;
            HRESULT hr=::CoCreateInstance(CLSID_CSoSmileyCtrl,NULL,CLSCTX_INPROC,__uuidof(ISoSmileyCtrl),(LPVOID*)&pSmiley); 
            if(FAILED(hr)) return 0;
            
            SComPtr<IRichEditOle> ole;
            if(SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)(void**)&ole) && ole)
            {
                SComPtr<IRichEditOleCallback> pCallback;
                hr=ole->QueryInterface(IID_IRichEditOleCallback,(void**)&pCallback);
                if(FAILED(hr)) return 0;
                SComPtr<ISmileyHost> host;
                hr = pCallback->QueryInterface(__uuidof(ISmileyHost),(void**)&host);
                if(FAILED(hr)) return 0;
                SComPtr<ISmileySource> pSource;
                hr = host->CreateSource(&pSource);
                if(FAILED(hr)) return 0;
                {
                    UINT uID = xmlText.attribute(L"id").as_uint(-1);
                    SStringW strPath = xmlText.attribute(L"path").value();
                    if(uID != -1)
                        hr = pSource->LoadFromID(uID);
                    else
                        hr = pSource->LoadFromFile(strPath);
                    if(SUCCEEDED(hr))
                    {
                        pSmiley->SetSource(pSource);
                        SSendMessage(EM_SETSEL,iCaret,iCaret);
                        pSmiley->Insert2Richedit((DWORD_PTR)(void*)ole);
                    }
                }
            }
            return SUCCEEDED(hr)?1:0;
        }
        
        CHARFORMAT cfNew = cf;
        cfNew.dwMask = 0;
        if(xmlText.name() == KLabelColor)
        {
            cfNew.crTextColor = StringToColor(xmlText.attribute(L"value").value()) & 0x00ffffff;
            cfNew.dwMask |= CFM_COLOR;
        }else if(xmlText.name()== KLabelFont)
        {
            wcscpy(cf.szFaceName,cfNew.szFaceName);
            wcscpy_s(cfNew.szFaceName,LF_FACESIZE-1,xmlText.attribute(L"value").value());
            cfNew.dwMask |= CFM_FACE;
        }else if(xmlText.name()==KLabelUnderline)
        {
            cfNew.dwMask |=CFM_UNDERLINE;
            cfNew.dwEffects |= CFE_UNDERLINE;
        }else if(xmlText.name() == KLabelItalic)
        {
            cfNew.dwMask |=CFM_ITALIC;
            cfNew.dwEffects |= CFE_ITALIC;
        }else if(xmlText.name() == KLabelBold)
        {
            cfNew.dwMask |=CFM_BOLD;
            cfNew.dwEffects |= CFE_BOLD;
        }else if(xmlText.name() == KLabelStrike)
        {
            cfNew.dwMask |= CFM_STRIKEOUT;
            cfNew.dwEffects |= CFE_STRIKEOUT;
        }else if(xmlText.name() == KLabelLink)
        {
            cfNew.dwMask |= CFM_LINK;
            cfNew.dwEffects |= CFE_LINK;
            COLORREF cr = StringToColor(xmlText.attribute(L"color").value());
            if(cr!=0)
            {
                cfNew.dwMask |= CFM_COLOR;
                cfNew.crTextColor = cr & 0x00ffffff;
            }
        }
        
        
        SStringT strTextT = S_CW2T(strText);

        int nRet = strTextT.GetLength();
        
        SSendMessage(EM_REPLACESEL,FALSE,(LPARAM)(LPCTSTR)strTextT);
        int iEnd = iCaret + nRet;
        SSendMessage(EM_SETSEL,iCaret,iEnd);
        SSendMessage(EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cfNew);
        iCaret = iEnd;
        SSendMessage(EM_SETSEL,iCaret,iCaret);
        
        pugi::xml_node xmlChild = xmlText.first_child();
        while(xmlChild)
        {
            int nSubLen = _AppendFormatText(iCaret,cfNew,xmlChild);
            iCaret += nSubLen;
            nRet += nSubLen;
            
            xmlChild = xmlChild.next_sibling();
        }
        if(cfNew.dwMask)
        {
            cf.dwMask = CFM_ALL;
            SSendMessage(EM_SETCHARFORMAT,SCF_SELECTION,(LPARAM)&cf);
        }
        return nRet;
    }

    int SChatEdit::OnCreate(LPVOID)
    {
        int nRet = SRichEdit::OnCreate(NULL);
        if(nRet == 0)
        {
            DWORD dwEvtMask = SSendMessage(EM_GETEVENTMASK);
            SSendMessage(EM_SETEVENTMASK,0,dwEvtMask | ENM_LINK);
        }
        return nRet;
    }

    void SChatEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if(nChar == VK_RETURN)
        {
            EventChatEditKeyReturn evt(this);
            if(evt.bCancel) return;
        }
        return SRichEdit::OnKeyDown(nChar,nRepCnt,nFlags);
    }
    SStringW SChatEdit::GetFormatText()
    {
        SStringW strMsg;


        TEXTRANGE  txtRng;
        txtRng.chrg.cpMin =0;
        txtRng.chrg.cpMax = SSendMessage(WM_GETTEXTLENGTH);
        txtRng.lpstrText = strMsg.GetBufferSetLength(txtRng.chrg.cpMax);
        
        SSendMessage(EM_GETTEXTRANGE,0,(LPARAM)&txtRng);
        strMsg.ReleaseBuffer();

        SComPtr<IRichEditOle> ole;
        SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)(void**)&ole);

        for(int i=0;i<strMsg.GetLength();i++)
        {
            if(strMsg[i] == 0xfffc)
            {//找到一个OLE对象
                REOBJECT reobj={sizeof(reobj),0};
                reobj.cp = i;
                HRESULT hr = ole->GetObject( REO_IOB_USE_CP , &reobj, REO_GETOBJ_POLEOBJ);
                if(SUCCEEDED(hr) && reobj.poleobj)
                {
                    if(reobj.clsid == CLSID_CSoSmileyCtrl)
                    {
                        SComPtr<ISoSmileyCtrl> smiley;
                        hr = reobj.poleobj->QueryInterface(__uuidof(ISoSmileyCtrl), (void**)&smiley);
                        if(SUCCEEDED(hr))
                        {
                            SComPtr<ISmileySource> source;
                            hr = smiley->GetSource(&source);
                            SASSERT(SUCCEEDED(hr));
                            UINT uID = -1;
                            SStringW strSmiley = L"<smiley ";
                            if(SUCCEEDED(source->GetID(&uID)))
                            {
                                strSmiley += SStringW().Format(L"id=\"%d\"",uID);
                            }

                            BSTR strFile;
                            if(SUCCEEDED(source->GetFile(&strFile)))
                            {
                                strSmiley += SStringW().Format(L"path=\"%s\"",strFile);
                                ::SysFreeString(strFile);
                            }
                            strSmiley += L"/>";

                            strMsg = strMsg.Left(i) + strSmiley + strMsg.Right(strMsg.GetLength() - i);

                            i += strSmiley.GetLength() -1;
                        }
                    }
                    reobj.poleobj->Release();
                }
            }
        }
        return strMsg;
    }


}

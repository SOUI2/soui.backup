#pragma once
#include "duiwnd.h"
#include "DuiRichEdit.h"
#include "DuiDropDown.h"
#include "duilistbox.h"
#include "duilistboxex.h"
#include "DuiCmnCtrl.h"

namespace SOUI
{

#define IDC_CB_EDIT			-100
#define IDC_DROPDOWN_LIST	-200

class CDuiComboBoxBase;

class CComboEdit:public CDuiEdit
{
public:
	CComboEdit(CDuiComboBoxBase *pOwner);
	virtual ~CComboEdit(){}
protected:
	void OnMouseHover(WPARAM wParam, CPoint ptPos);
	
	void OnMouseLeave();

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	virtual LRESULT DuiNotify(LPDUINMHDR pnms);

	WND_MSG_MAP_BEGIN()
		MSG_WM_MOUSEHOVER(OnMouseHover)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_KEYDOWN(OnKeyDown)
	WND_MSG_MAP_END()
};

class SOUI_EXP CDuiComboBoxBase 
	: public CDuiWindow
	, public IDuiDropDownOwner
{
	friend class CComboListBox;
public:
	CDuiComboBoxBase(void);
	virtual ~CDuiComboBoxBase(void);

	virtual int GetCurSel() const =0;

	virtual CDuiStringT GetWindowText() =0;

	void DropDown();
	void CloseUp();
protected:
	//////////////////////////////////////////////////////////////////////////
	//	CDuiDropDownOwner
	virtual CDuiWindow* GetDropDownOwner();
	virtual void OnDropDown(CDuiDropDownWnd *pDropDown);
	virtual void OnCloseUp(CDuiDropDownWnd *pDropDown,UINT uCode);
protected:
	virtual void OnSelChanged() =0;

	virtual LRESULT DuiNotify(LPDUINMHDR pnms);
protected:
	//计算弹出窗口位置
	BOOL CalcPopupRect(int nHeight,CRect & rcPopup);
	virtual BOOL CreateListBox(pugi::xml_node xmlNode)=0;
	virtual int  GetListBoxHeight()=0;

	void GetDropBtnRect(LPRECT prc);
	virtual BOOL LoadChildren(pugi::xml_node xmlNode);	
	virtual void GetTextRect(LPRECT pRect);

	void OnPaint(CDCHandle dc);
	void OnLButtonDown(UINT nFlags,CPoint pt);
	void OnMouseMove(UINT nFlags,CPoint pt);
	void OnMouseLeave();
	void OnKeyDown( TCHAR nChar, UINT nRepCnt, UINT nFlags );
	void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnDestroy();

	UINT OnGetDuiCode();
	BOOL IsTabStop();

	SOUO_ATTRIBUTES_BEGIN()
		DUIWIN_INT_ATTRIBUTE("dropdown", m_bDropdown, FALSE)
		DUIWIN_INT_ATTRIBUTE("dropheight", m_nDropHeight, FALSE)
 		DUIWIN_INT_ATTRIBUTE("cursel", m_iInitSel, FALSE)
		DUIWIN_SKIN_ATTRIBUTE("btnskin", m_pSkinBtn, FALSE)
		DUIWIN_INT_ATTRIBUTE("animtime", m_iAnimTime, FALSE)
	SOUI_ATTRIBUTES_END()

	WND_MSG_MAP_BEGIN()
		MSG_WM_PAINT(OnPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)		
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_KEYDOWN(OnKeyDown) 
		MSG_WM_CHAR(OnChar)
		MSG_WM_DESTROY(OnDestroy)
	WND_MSG_MAP_END()

protected:
	CDuiStringT GetEditText()
	{
		if(!m_bDropdown)
		{
			int nLen=m_pEdit->GetWindowTextLength();
			wchar_t *pszBuf=new wchar_t[nLen+1];
			m_pEdit->GetWindowText(pszBuf,nLen);
			pszBuf[nLen]=0;
			CDuiStringT str=DUI_CW2T(pszBuf);
			delete pszBuf;
			return str;
		}else
		{
			return CDuiStringT();
		}
	}

	CDuiRichEdit *m_pEdit;
	CDuiSkinBase *m_pSkinBtn;
	DWORD		  m_dwBtnState;
	
	BOOL m_bDropdown;
	int  m_nDropHeight;
	int  m_iAnimTime;	
	int	 m_iInitSel;
	CDuiDropDownWnd *m_pDropDownWnd;
};

class SOUI_EXP CDuiComboBox : public CDuiComboBoxBase
{
	SOUI_CLASS_NAME(CDuiComboBox, "combobox")
public:
	CDuiComboBox();
	virtual ~CDuiComboBox();

	BOOL SetCurSel(int iSel)
	{
		if(m_pListBox->SetCurSel(iSel))
		{
			OnSelChanged();
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	int GetCurSel() const
	{
		return m_pListBox->GetCurSel();
	}

	int  GetCount()
	{
		return m_pListBox->GetCount();
	}

	CDuiStringT GetWindowText()
	{
		if(!m_bDropdown)
		{
			return GetEditText();
		}
		if(m_pListBox->GetCurSel()==-1) return _T("");
		return GetLBText(m_pListBox->GetCurSel());
	}

	LPARAM GetItemData(UINT iItem) const
	{
		return m_pListBox->GetItemData(iItem);
	}

	int SetItemData(UINT iItem, LPARAM lParam)
	{
		return m_pListBox->SetItemData(iItem,lParam);
	}

	int InsertItem(UINT iPos,LPCTSTR pszText,int iIcon,LPARAM lParam)
	{
		return m_pListBox->InsertString(iPos,pszText,iIcon,lParam);
	}

	BOOL DeleteString(UINT iItem)
	{
		return m_pListBox->DeleteString(iItem);
	}

	void ResetContent()
	{
		return m_pListBox->DeleteAll();
	}

	CDuiStringT GetLBText(int iItem)
	{
		CDuiStringT strText;
		m_pListBox->GetText(iItem,strText);
		return strText;
	}

	CDuiListBox * GetListBox(){return m_pListBox;}
protected:
	virtual BOOL CreateListBox(pugi::xml_node xmlNode);
	virtual int  GetListBoxHeight();

	virtual void OnDropDown(CDuiDropDownWnd *pDropDown);

	virtual void OnCloseUp(CDuiDropDownWnd *pDropDown,UINT uCode);

	virtual void OnSelChanged();

protected:

	CDuiListBox *m_pListBox;
};

class SOUI_EXP CDuiComboBoxEx : public CDuiComboBoxBase
{
	SOUI_CLASS_NAME(CDuiComboBox, "comboboxex")
public:
	CDuiComboBoxEx();
	virtual ~CDuiComboBoxEx();

	BOOL SetCurSel(int iSel)
	{
		if(m_pListBox->SetCurSel(iSel))
		{
			OnSelChanged();
			return TRUE;
		}else
		{
			return FALSE;
		}
	}

	int GetCurSel() const
	{
		return m_pListBox->GetCurSel();
	}

	int  GetCount()
	{
		return m_pListBox->GetItemCount();
	}

	CDuiStringT GetWindowText()
	{
		if(!m_bDropdown) return GetEditText();
		return GetLBText(m_pListBox->GetCurSel());
	}

	LPARAM GetItemData(UINT iItem) const
	{
		return m_pListBox->GetItemData(iItem);
	}

	void SetItemData(UINT iItem, LPARAM lParam)
	{
		m_pListBox->SetItemData(iItem,lParam);
	}

	int InsertItem(UINT iPos,LPCTSTR pszText,int iIcon,LPARAM lParam)
	{
		int iInserted= m_pListBox->InsertItem(iPos,NULL,lParam);
		if(iInserted!=-1)
		{
			CDuiWindow *pWnd=m_pListBox->GetItemPanel(iInserted);
			if(m_uTxtID!=0)
			{
				CDuiWindow *pText=pWnd->FindChildByCmdID(m_uTxtID);
				if(pText) pText->SetInnerText(pszText);
			}
			if(m_uIconID!=0)
			{
				CDuiImageWnd *pIcon=pWnd->FindChildByCmdID2<CDuiImageWnd*>(m_uIconID);
				if(pIcon) pIcon->SetIcon(iIcon);
			}
		}
		return iInserted;
	}

	void DeleteString(UINT iItem)
	{
		m_pListBox->DeleteItem(iItem);
	}

	void ResetContent()
	{
		return m_pListBox->DeleteAllItems();
	}

	CDuiStringT GetLBText(int iItem)
	{
		if(m_uTxtID == 0 || iItem<0 || iItem>= GetCount()) return _T("");
		CDuiWindow *pItem=m_pListBox->GetItemPanel(iItem);
		CDuiWindow *pText=pItem->FindChildByCmdID(m_uTxtID);
		if(!pText) return _T("");
		return pText->GetInnerText();
	}

	CDuiListBoxEx * GetListBox(){return m_pListBox;}

protected:
	virtual void OnSelChanged();
protected:
	virtual BOOL CreateListBox(pugi::xml_node xmlNode);
	virtual int  GetListBoxHeight();

	virtual void OnDropDown(CDuiDropDownWnd *pDropDown);

	virtual void OnCloseUp(CDuiDropDownWnd *pDropDown,UINT uCode);

protected:

	SOUO_ATTRIBUTES_BEGIN()
		DUIWIN_UINT_ATTRIBUTE("id_text", m_uTxtID, FALSE)
		DUIWIN_UINT_ATTRIBUTE("id_icon", m_uIconID, FALSE)
	SOUI_ATTRIBUTES_END()

	CDuiListBoxEx *m_pListBox;
	UINT	m_uTxtID,m_uIconID;
};

}//namespace
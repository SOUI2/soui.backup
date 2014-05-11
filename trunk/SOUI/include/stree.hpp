// STreeT.hpp: interface for the CSTree class.
//	树模板：提供对一般的数据类型实现树结构
//	所有操作类似MFC的CTreeCtrl，只是没有显示功能
//	黄建雄(haungjianxiong@sina.com)
//	version: 1.0　2003-10-24	实现基本功能
//			 2.0  2004-12-29	增加两个遍历接口，修改内存释放部分可能存在的bug
//			 2.1  2006-10-17	为节点增加一个hChildLast数据,以加快在数据插入时的速度
//			 2.2  2008-10-16    修改一个遍历接口的问题
//			 2.3  2011-10-17	将数据释放的接口从回调函数改成虚函数
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREE_H__D2332B4E_2C7E_4357_BE22_EC55BF496C1C__INCLUDED_)
#define AFX_STREE_H__D2332B4E_2C7E_4357_BE22_EC55BF496C1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef DUIASSERT
#define DUIASSERT(x)
#endif

#include "DuiDef.h"
//typedef ULONG_PTR HSTREEITEM;

#define STVI_ROOT	((HSTREEITEM)0xFFFF0000)//=TVI_ROOT
#define STVI_FIRST	((HSTREEITEM)0xFFFF0001)//=TVI_FIRST
#define STVI_LAST	((HSTREEITEM)0xFFFF0002)//=TVI_LAST

#define STVN_ROOT	((HSTREENODE)(ULONG_PTR)0xFFFF0000)//=STVN_ROOT
#define STVN_FIRST	((HSTREENODE)(ULONG_PTR)0xFFFF0001)//=STVN_FIRST
#define STVN_LAST	((HSTREENODE)(ULONG_PTR)0xFFFF0002)//=STVN_LAST

#define STVL_ROOT	((PSTREELINK)0xFFFF0000)

template<class T>
class SOUI_EXP CSTree  
{
	typedef struct _STREENODE{
		struct _STREENODE *	hParent;
		struct _STREENODE *	hChildFirst;
		struct _STREENODE * hChildLast;
		struct _STREENODE *	hPrevSibling;
		struct _STREENODE *	hNextSibling;
		T					data;
	}STREENODE,*HSTREENODE;//树结点

	//树结点的链接
	typedef struct _STREELINK{
		HSTREENODE	hParent;
		HSTREENODE	hChildFirst;
		HSTREENODE  hChildLast;
		HSTREENODE	hPrevSibling;
		HSTREENODE	hNextSibling;
	}STREELINK,*PSTREELINK;

	//**********************************************************
	//	遍历一个树结点的回调函数原型，
	//	T *:当前遍历到的结点的数据
	//	LPARAM:回调时使用的参数
	//	return:FALSE－继续，TRUE－中断遍历
	//**********************************************************
	typedef BOOL (*CBTRAVERSING)(T*,LPARAM);

public:
	CSTree()
	{
		m_hRootFirst=NULL;
		m_hRootLast=NULL;
	}
	virtual ~CSTree(){DeleteAllItems();}

	void DeleteAllItems(){
		if(m_hRootFirst)
		{
			FreeNode(STVN_ROOT);
			m_hRootFirst=NULL;
			m_hRootLast=NULL;
		}
	}
	
	//*********************************************
	//	获取下一个兄弟
	//*********************************************
	static HSTREEITEM GetNextSiblingItem(HSTREEITEM hItem){
		PSTREELINK pLink= (PSTREELINK)hItem;
		DUIASSERT(pLink&&pLink!=STVL_ROOT);
		return (HSTREEITEM)pLink->hNextSibling;
	}

	//***********************************************
	//	获取上一个兄弟
	//*********************************************
	static HSTREEITEM GetPrevSiblingItem(HSTREEITEM hItem)
	{
		PSTREELINK pLink= (PSTREELINK)hItem;
		DUIASSERT(pLink&&pLink!=STVL_ROOT);
		return (HSTREEITEM)pLink->hPrevSibling;
	}
	
	//*********************************************
	//　获取父结点
	//*********************************************
	static HSTREEITEM GetParentItem(HSTREEITEM hItem)
	{
		PSTREELINK pLink= (PSTREELINK)hItem;
		DUIASSERT(pLink&&pLink!=STVL_ROOT);
		return (HSTREEITEM)pLink->hParent;
	}
	

	//*********************************************
	//　获取结点层数
	//*********************************************
	static int GetItemLevel(HSTREEITEM hItem)
	{
		int nRet=-1;
		if(hItem==STVI_ROOT) hItem=NULL;
		while(hItem)
		{
			nRet++;
			hItem=GetParentItem(hItem);
		}
		return nRet;
	}

	//************************************************
	//	获取第一个子结点
	//************************************************
	HSTREEITEM GetChildItem(HSTREEITEM hItem,BOOL bFirst=TRUE)
	{
		HSTREENODE hsNode= (HSTREENODE)hItem;
		DUIASSERT(hsNode);
		if(hsNode==STVN_ROOT)
		{
			if(bFirst)
				return (HSTREEITEM)m_hRootFirst;
			else
				return (HSTREEITEM)m_hRootLast;
		}
		else
		{
			if(bFirst)
				return (HSTREEITEM)hsNode->hChildFirst;
			else
				return (HSTREEITEM)hsNode->hChildLast;
		}
	}
	
	//************************************************
	//	获取子结点数量
	//************************************************
	int GetChildrenCount(HSTREEITEM hItem)
	{
		int nRet=0;
		HSTREEITEM hChild=GetChildItem(hItem);
		while(hChild)
		{
			nRet++;
			hChild=GetNextSiblingItem(hChild);
		}
		return nRet;
	}

	//************************************************
	//	删除一个节点，可以被派生类重载
	//***********************************************
	virtual void DeleteItem(HSTREEITEM hItem)
	{
		HSTREENODE hsNode= (HSTREENODE)hItem;
		DUIASSERT(hsNode);
		if(hsNode==STVN_ROOT)
		{
			FreeNode(STVN_ROOT);
			m_hRootFirst=NULL;
			m_hRootLast=NULL;
			return;
		}
		STREENODE nodeCopy=*hsNode;
		BOOL bRootFirst=hsNode==m_hRootFirst;
		BOOL bRootLast=hsNode==m_hRootLast;
		FreeNode(hsNode);

		if(nodeCopy.hPrevSibling)//has prevsibling
			nodeCopy.hPrevSibling->hNextSibling=nodeCopy.hNextSibling;
		else if(nodeCopy.hParent)//parent's first child
			nodeCopy.hParent->hChildFirst=nodeCopy.hNextSibling;
		if(nodeCopy.hNextSibling)// update next sibling's previous sibling
			nodeCopy.hNextSibling->hPrevSibling=nodeCopy.hPrevSibling;
		else if(nodeCopy.hParent)//parent's last child
			nodeCopy.hParent->hChildLast=nodeCopy.hPrevSibling;
		//update root item
		if(bRootFirst)	m_hRootFirst=nodeCopy.hNextSibling;
		if(bRootLast)   m_hRootLast=nodeCopy.hPrevSibling;
	}


	//******************************************
	//	删除一个结点分枝，如果该结点的父结点没有其它子节点则一起删除
	//******************************************
	BOOL DeleteItemEx(HSTREEITEM hItem)
	{
		if(GetChildItem(hItem)) return FALSE;
		while(hItem && !GetChildItem(hItem))
		{
			HSTREEITEM hParent=GetParentItem(hItem);
			DeleteItem(hItem);
			hItem=hParent;
		}
		return TRUE;
	}

	//******************************************
	//	获取结点中保存的数据
	//******************************************
	static T GetItem(HSTREEITEM hItem){
		DUIASSERT(hItem!=STVI_ROOT);
		HSTREENODE hsNode= (HSTREENODE)hItem;
		DUIASSERT(hsNode);
		return hsNode->data;
	}
	

	static T *GetItemPt(HSTREEITEM hItem){
		DUIASSERT(hItem!=STVI_ROOT);
		HSTREENODE hsNode= (HSTREENODE)hItem;
		DUIASSERT(hsNode);
		return &hsNode->data;
	}
	//*******************************************
	//	插入一个新结点
	//	data:结点数据
	//	hParant:新结点的父结点
	//	hInsertAfter:新结点的前一个兄弟结点
	//	return:新结点的指针
	//*******************************************
	HSTREEITEM InsertItem(const T &data,HSTREEITEM hParent=STVI_ROOT,HSTREEITEM hInsertAfter=STVI_LAST){
		HSTREENODE hParentNode=(HSTREENODE) hParent;
		HSTREENODE hInsertAfterNode=(HSTREENODE) hInsertAfter;
		if(hParentNode==STVN_ROOT)
			hParentNode=NULL;
		DUIASSERT(hInsertAfter);
		if(hInsertAfterNode!=STVN_FIRST && hInsertAfterNode!=STVN_LAST)
		{
			if(hInsertAfterNode->hParent!=hParentNode) return NULL;
			if(hInsertAfterNode->hNextSibling==NULL) hInsertAfterNode=STVN_LAST;
		}

		HSTREENODE hInserted=new STREENODE;
		hInserted->data=data;
		hInserted->hParent=hParentNode;
		hInserted->hChildFirst=NULL;
		hInserted->hChildLast=NULL;

		if(hInsertAfterNode==STVN_FIRST)
		{
			hInserted->hPrevSibling=NULL;
			if(hParentNode==NULL)//root
			{
				hInserted->hNextSibling=m_hRootFirst;
				if(m_hRootFirst) m_hRootFirst->hPrevSibling=hInserted;
				m_hRootFirst=hInserted;
				if(m_hRootLast==NULL) m_hRootLast=hInserted;
			}else	//has parent
			{
				hInserted->hNextSibling=hParentNode->hChildFirst;
				if(hInserted->hNextSibling)
				{
					hInserted->hNextSibling->hPrevSibling=hInserted;
					hParentNode->hChildFirst=hInserted;
				}else
				{
					hParentNode->hChildLast=hParentNode->hChildFirst=hInserted;
				}
			}
		}else if(hInsertAfterNode==STVN_LAST)
		{
			hInserted->hNextSibling=NULL;
			if(hParentNode==NULL)//root
			{
				hInserted->hPrevSibling=m_hRootLast;
				if(m_hRootLast) m_hRootLast->hNextSibling=hInserted;
				m_hRootLast=hInserted;
				if(!m_hRootFirst) m_hRootFirst=hInserted;
			}else
			{
				hInserted->hPrevSibling=hParentNode->hChildLast;
				if(hParentNode->hChildLast) 
				{
					hInserted->hPrevSibling->hNextSibling=hInserted;
					hParentNode->hChildLast=hInserted;
				}else
				{
					hParentNode->hChildLast=hParentNode->hChildFirst=hInserted;
				}				
			}
		}else
		{
			HSTREENODE hNextSibling=hInsertAfterNode->hNextSibling;
			hInserted->hPrevSibling=hInsertAfterNode;
			hInserted->hNextSibling=hNextSibling;
			hNextSibling->hPrevSibling = hInsertAfterNode->hNextSibling = hInserted;
		}
		return (HSTREEITEM)hInserted;
	}

	//********************************************************
	//	采用递归方式遍历一个树结点
	//	HSTREEITEM hItem:起始结点
	//	CBTRAVERSING funTraversing:执行实际操作的回调函数
	//	LPARAM lParam:回调时使用的参数
	//********************************************************
	HSTREEITEM TraversingRecursion(HSTREEITEM hItem,CBTRAVERSING funTraversing,LPARAM lParam)
	{
		DUIASSERT(hItem);
		if(hItem!=STVI_ROOT)
		{
			if(funTraversing(GetItemPt(hItem),lParam)) return hItem;
		}
		HSTREEITEM hChild=GetChildItem(hItem);
		while(hChild)
		{
			HSTREEITEM hTmp=GetChildItem(hChild);
			if(hTmp)
			{
				HSTREEITEM hRet=TraversingRecursion(hTmp,funTraversing,lParam);
				if(hRet) return hRet;
			}else
			{
				if(funTraversing(GetItemPt(hChild),lParam)) return hChild;
			}
			hChild=GetNextSiblingItem(hChild);
		}
		return NULL;
	}
	
	//********************************************************
	//	按顺序方式从指定结点开始查找后面的结点，包括自己的子节点及自己向下的兄弟结点
	//	HSTREEITEM hItem:起始结点
	//	CBTRAVERSING funTraversing:执行实际操作的回调函数
	//	LPARAM lParam:回调时使用的参数
	//	remark:在执行顺序查找时需要这种方式
	//********************************************************
	HSTREEITEM TraversingSequence(HSTREEITEM hItem,CBTRAVERSING funTraversing,LPARAM lParam)
	{
		if(!m_hRootFirst) return NULL;
		if(hItem!=STVI_ROOT)
		{
			if(funTraversing(GetItemPt(hItem),lParam)) return hItem;
		}
		HSTREEITEM hNext=GetNextItem(hItem);
		while(hNext)
		{
			if(funTraversing(GetItemPt(hNext),lParam)) return hNext;
			hNext=GetNextItem(hNext);
		}
		return NULL;
	}

	HSTREEITEM GetRootItem(BOOL bFirst=TRUE){
		return (HSTREEITEM)(bFirst?m_hRootFirst:m_hRootLast);
	}

	//******************************************
	//	获取结点的子孙结点数
	//******************************************
	int GetDesendants(HSTREEITEM hItem)
	{
		int nRet=0;
		HSTREEITEM hChild=GetChildItem(hItem);
		while(hChild)
		{
			nRet += 1+GetDesendants(hChild);
			hChild=GetNextSiblingItem(hChild);
		}
		return nRet;
	}

	//****************************************************
	//	获取当前结点的下一个结点
	//	HSTREEITEM hItem:当前结点
	//	return:当前结点的下一个结点
	//	remark：如果当前结点有子结点，则返回自己的第一个子结点，
	//			否则如果有向下的兄弟结点，则返回自己向下兄弟结点、
	//			否则搜索自己的父结点的向下兄弟结点
	//****************************************************
	HSTREEITEM GetNextItem(HSTREEITEM hItem)
	{
		if(hItem==STVI_ROOT) return (HSTREEITEM)m_hRootFirst;

		HSTREEITEM hRet=GetChildItem(hItem);
		if(hRet) return hRet;
		HSTREEITEM hParent=hItem;
		while(hParent)
		{
			hRet=GetNextSiblingItem(hParent);
			if(hRet) return hRet;
			hParent=GetParentItem(hParent);
		}
		return NULL;
	}

	//****************************************************
	//	获取当前结点的下一个结点
	//	HSTREEITEM hItem:当前结点
	//	int &nLevel:当前结点(hItem)与目标结点(return)的层次关系,1-父子关系，0－兄弟关系，-n－子->父的兄弟
	//	return:当前结点的下一个结点
	//	remark：如果当前结点有子结点，则返回自己的第一个子结点，
	//			否则如果有向下的兄弟结点，则返回自己向下兄弟结点、
	//			否则搜索自己的父结点的向下兄弟结点
	//****************************************************
	HSTREEITEM GetNextItem(HSTREEITEM hItem,int &nLevel)
	{
		if(hItem==STVI_ROOT)
		{
			nLevel=1;
			return (HSTREEITEM)m_hRootFirst;
		}

		HSTREEITEM hRet=GetChildItem(hItem);
		if(hRet)
		{
			nLevel=1;
			return hRet;
		}
		HSTREEITEM hParent=hItem;
		nLevel=0;
		while(hParent)
		{
			hRet=GetNextSiblingItem(hParent);
			if(hRet) return hRet;
			nLevel--;
			hParent=GetParentItem(hParent);
		}
		return NULL;
	}
private:
	//**********************************************
	//	采用后序遍历的方式释放结点占用的空间。
	//**********************************************
	void FreeNode(HSTREENODE hsNode)
	{
		DUIASSERT(hsNode);
		HSTREENODE hSibling=(HSTREENODE)GetChildItem((HSTREEITEM)hsNode);
		while(hSibling)
		{
			HSTREENODE hNextSibling=hSibling->hNextSibling;
			FreeNode(hSibling);
			hSibling=hNextSibling;
		}
		if(hsNode!=STVN_ROOT)
		{
			OnNodeFree(hsNode->data);
			delete hsNode;
		}
	}

protected:
	//在派生类中实现数据的释放操作
	virtual void OnNodeFree(T & data){}

	HSTREENODE	m_hRootFirst;	//第一个根节点
	HSTREENODE  m_hRootLast;	//最后一个根节点
};

#endif // !defined(AFX_STREE_H__D2332B4E_2C7E_4357_BE22_EC55BF496C1C__INCLUDED_)

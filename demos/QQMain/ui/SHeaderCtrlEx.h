/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 * 
 * @file       SHeaderCtrlEx.h
 * @brief      
 * @version    v1.0      
 * @author     008 
 * @date       2017-01-07
 * 
 * Describe     
 */
#pragma once


#include "control/SHeaderCtrl.h"
namespace SOUI
{
  /**
   * @class     SHeaderCtrlEx
   * @brief     表头控件 
   * 
   * Describe   表头控件
   */
  class SHeaderCtrlEx: public SHeaderCtrl
  {
      SOUI_CLASS_NAME(SHeaderCtrlEx, L"header2")
  public:     
      SHeaderCtrlEx(void);

      SOUI_ATTRS_BEGIN()         
		  ATTR_INT(L"matchParent",m_bMatchParent,FALSE)
      SOUI_ATTRS_END()
  protected:
	  //添加一个函数支持同父窗口变化自身大小，些时和width按照它们的值等比例化分
	  bool isViewWidthMatchParent() const
	  {
		  return TRUE==m_bMatchParent;
	  }
	  void UpdataWidth()
	  {
		  //更新不同步
		  SASSERT(m_widItems.GetCount() == m_arrItems.GetCount());
		  int totalWidth=0;
		  for (unsigned int i = 0;i < m_widItems.GetCount();i++)
		  {
			  totalWidth += m_widItems[i];
		  }
		  int parentViewWid =GetWindowRect().Width();
		  int remainingWid= parentViewWid;
		  int nozeroPos = 0;//找到第一个不为0的项
		  for (unsigned int i =0;i < m_arrItems.GetCount();i++)
		  {
			  //跳过为0的项
			  if (m_widItems[i] == 0)
				  continue;
			  nozeroPos = i;
			  break;
		  }
		  for (int i = m_arrItems.GetCount()-1;i >nozeroPos;i--)
		  {
			  //跳过为0的项
			  if(m_widItems[i]==0)
				  continue;
			  int itemwid=m_widItems[i]*parentViewWid/totalWidth;
			  m_arrItems[i].cx.fSize = itemwid;
			  remainingWid -= itemwid;
		  }
		  //因为除法不一定能整除，把最后余下的全给第一个不为0的子项
		  m_arrItems[nozeroPos].cx.fSize = remainingWid;
	  }
	  void OnSize(UINT nType, CSize size)
	  {
		  if (isViewWidthMatchParent())
			  UpdataWidth();
	  }   
      virtual BOOL CreateChildren(pugi::xml_node xmlNode);
	  virtual BOOL OnSetCursor(const CPoint &pt);
      SOUI_MSG_MAP_BEGIN() 
		  MSG_WM_SIZE(OnSize)
      SOUI_MSG_MAP_END()      
	  SArray<int> m_widItems; /**< 列表项集合 */
	  BOOL m_bMatchParent;
  };
}//end of namespace SOUI

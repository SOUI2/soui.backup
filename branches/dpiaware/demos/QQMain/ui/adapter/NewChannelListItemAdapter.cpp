#include "stdafx.h"
#include "ui/adapter/NewChannelListItemAdapter.h"
#include "ui/SToggleEx.h"

#include "res/R.h" 
 
namespace SOUI
{  

const wchar_t * KNewChannelAttrName_Height[] = {
	L"defHeight",
	L"friendItemHeight" 
};

CNewChannelListItemAdapter::CNewChannelListItemAdapter(int data, live::IListItemCallback *pCB)
{ 
	this->SetCallback(data, pCB);

	root_node_ = new Node<NewChannelInfo>();
    root_node_->data().level_ = -1;
    root_node_->data().child_visible_ = true;
    root_node_->data().has_child_ = true; 
}

CNewChannelListItemAdapter::~CNewChannelListItemAdapter()
{
    Clear();
} 

void CNewChannelListItemAdapter::RemoveAll()
{ 
    for (int i = 0; i < root_node_->num_children(); ++i)
    {
		Node<NewChannelInfo>* child = root_node_->child(i);
        RemoveNode(child);
    }
    delete root_node_;

	root_node_ = new Node<NewChannelInfo>();
    root_node_->data().level_ = -1;
    root_node_->data().child_visible_ = true;
    root_node_->data().has_child_ = true; 
} 

Node<NewChannelInfo>* CNewChannelListItemAdapter::GetRoot()
{
    return root_node_;
}
 
Node<NewChannelInfo>* CNewChannelListItemAdapter::AddGroupNode(const std::wstring &title, bool has_child, Node<NewChannelInfo>* parent)
{
	if (!parent)
		parent = root_node_;


	Node<NewChannelInfo>* node = new Node<NewChannelInfo>();

	node->data().id_ = m_Nodes.getSeqence();
	node->data().level_ = parent->data().level_ + 1;
	node->data().has_child_ = has_child;
	 
	node->data().folder_ = true;

	node->data().child_visible_ = (node->data().level_ == 0);
	node->data().child_visible_ = true;

	node->data().text_ = title;
	node->data().value = NULL;

	parent->add_child(node);
	 
	m_Nodes.AddID(node->data().id_, node);
	return node;
}
 
Node<NewChannelInfo>* CNewChannelListItemAdapter::AddNode(const std::wstring &title, unsigned int  channel_id, bool has_child, Node<NewChannelInfo>* parent)
{
    if (!parent)
        parent = root_node_;  
	 
	NewChannelInfo *pUserData = new NewChannelInfo();
	pUserData->channel_id = channel_id;

	Node<NewChannelInfo> *node = new Node<NewChannelInfo>;

	node->data().id_ = m_Nodes.getSeqence();
    node->data().level_ = parent->data().level_ + 1;
    node->data().has_child_ = has_child; 
	node->data().folder_ = has_child;

    node->data().child_visible_ = (node->data().level_ == 0);
    node->data().child_visible_ = false;

	node->data().text_ = title;
	node->data().value = pUserData;

    parent->add_child(node);

	m_Nodes.AddID(node->data().id_, node);
    return node;
}

bool CNewChannelListItemAdapter::RemoveNode(Node<NewChannelInfo>* node)
{
    if (!node || node == root_node_) return false;

    for (int i = 0; i < node->num_children(); ++i)
    {
		Node<NewChannelInfo>* child = node->child(i);
        RemoveNode(child);
    }
 
    node->parent()->remove_child(node);
    delete node;

    return true;
}

void CNewChannelListItemAdapter::SetChildVisible(Node<NewChannelInfo>* node, bool visible)
{
    if (!node || node == root_node_)
        return;

    if (node->data().child_visible_ == visible)
        return;

    node->data().child_visible_ = visible;

    TCHAR szBuf[MAX_PATH] = {0};
    std::wstring html_text;
    if (node->data().has_child_)
    {
        if (node->data().child_visible_)
            html_text += level_expand_image_;
        else
            html_text += level_collapse_image_;
		 
        html_text += szBuf;

        html_text += node->data().text_; 
    }
	 
    if (!node->has_children())
        return;

	if (visible)
	{
		int child_count = node->num_children();
		int prior_id = node->data().id_;
		for (int i = 0; i <child_count; ++i)
		{
			Node<NewChannelInfo> *pNode = node->child(i);
			m_Nodes.SetVisible(prior_id, pNode->data().id_);

			prior_id = pNode->data().id_;
		}  
	}
	else
	{
		int child_count = node->num_children();

		for (int i = 0; i <child_count; ++i)
		{
			Node<NewChannelInfo> *pNode = node->child(i);
			m_Nodes.SetInVisible(node->data().id_, pNode->data().id_);
		} 
	}
}

bool CNewChannelListItemAdapter::CanExpand(Node<NewChannelInfo>* node) const
{
    if (!node || node == root_node_)
        return false;

    return node->data().has_child_;
}

void CNewChannelListItemAdapter::SetCallback(int data, live::IListItemCallback *pCB)
{
	m_cb_data = data;
	m_pCB = pCB;
}
    
void CNewChannelListItemAdapter::Clear()
{
	RemoveAll();

    if (root_node_)
        delete root_node_;

    root_node_ = NULL;

} 

void CNewChannelListItemAdapter::InitByTemplate(pugi::xml_node xmlTemplate)
{
	m_nItemHeight[0] = xmlTemplate.attribute(KNewChannelAttrName_Height[0]).as_int(30);
	m_nItemHeight[1] = xmlTemplate.attribute(KNewChannelAttrName_Height[1]).as_int(60); 
}
 
int CNewChannelListItemAdapter::EnsureKeyVisible(BOOL bExpired, unsigned int  id)
{
    if(!bExpired)
    { 
        notifyDataSetChanged(); 
        return  0;
    }
	else
    { 
        notifyDataSetChanged(); 
		return 0;
    }
}
    
int CNewChannelListItemAdapter::getCount()
{
	return m_Nodes.GetCachelayerCount();
}

Node<NewChannelInfo> *CNewChannelListItemAdapter::GetNode(int id)
{
	return (Node<NewChannelInfo> *)m_Nodes.getNode(id);
}

int CNewChannelListItemAdapter::getItemViewType(int position)
{
	Node<NewChannelInfo> *pNode = GetNode(position);
	if (pNode && pNode->data().folder_)
        return VT_GROUP;
    else
        return VT_DATA;
}

SIZE CNewChannelListItemAdapter::getViewDesiredSize(int position, SWindow *pItem, LPCRECT prcContainer)
{ 
//	Node<NewChannelInfo> *pNode = GetNode(position);
	int viewType = getItemViewType(position);
	return CSize(0, m_nItemHeight[viewType]);//cx在listview，mclistview中没有使用，不需要计算
}
 
int CNewChannelListItemAdapter::getViewTypeCount()
{
    return 2;
}

SStringW CNewChannelListItemAdapter::GetColumnName(int iCol) const
{
    SStringW colNames[] = {L"col_nick"};
    return colNames[iCol];
}

void CNewChannelListItemAdapter::getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
{
	int nViewType = VT_GROUP;
	Node<NewChannelInfo> *pNode = GetNode(position);
	if (NULL == pNode)
		return;

	if (pNode && pNode->data().folder_)
		nViewType=  VT_GROUP;
	else
		nViewType = VT_DATA;
	 
    if(pItem->GetChildrenCount() == 0)
    {
        pugi::xml_node xmlItem;
        switch(nViewType)
        {
			case VT_GROUP: xmlItem = xmlTemplate.child(L"item_group");break;
			case VT_DATA: xmlItem = xmlTemplate.child(L"item_data");break;
        }
        pItem->InitFromXml(xmlItem);
    }

    if(nViewType == VT_GROUP)
    {
		SToggleEx * pSwitch = pItem->FindChildByID2<SToggleEx>(R.id.tgl_tv_expand);
		pSwitch->SetToggle(pNode->data().child_visible_);
		
		//因为点击toggle的时候，同时会触发EVT_ITEMPANEL_CLICK，因此，就直接用EVT_ITEMPANEL_CLICK就好了 
		pItem->SetUserData((ULONG_PTR)pNode);
		pItem->FindChildByID(R.id.txt_group)->SetWindowText(pNode->data().text_.c_str());
		pItem->GetEventSet()->unsubscribeEvent(EVT_ITEMPANEL_CLICK, Subscriber(&CNewChannelListItemAdapter::OnGroupClick, this));
		pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_CLICK, Subscriber(&CNewChannelListItemAdapter::OnGroupClick, this));
		 
    }
	else
    {
		pItem->SetUserData((ULONG_PTR)pNode);
		if(pNode->data().child_visible_)
		{   
			pItem->FindChildByID(R.id.txt_name)->SetWindowText(pNode->data().text_.c_str());

			NewChannelInfo *pItemData = pNode->data().value;
			wchar_t sUID[128];
			swprintf(sUID, L"%d", pItemData->uid);

			pItem->FindChildByID(R.id.txt_desc)->SetWindowText(sUID);

			pItem->GetEventSet()->unsubscribeEvent(EVT_ITEMPANEL_DBCLICK, Subscriber(&CNewChannelListItemAdapter::OnItemDblClick, this));
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_DBCLICK, Subscriber(&CNewChannelListItemAdapter::OnItemDblClick, this));
		}
		else
		{ 
			pItem->FindChildByID(R.id.txt_name)->SetWindowText(pNode->data().text_.c_str());			
			

			NewChannelInfo *pItemData = pNode->data().value;
			wchar_t sUID[128];
			swprintf(sUID, L"%d", pItemData->uid);

			pItem->FindChildByID(R.id.txt_desc)->SetWindowText(sUID);

		}
    }
}

void CNewChannelListItemAdapter::ExpendNode(int position, bool bChildVisible)
{
	Node<NewChannelInfo> *pNode = GetNode(position);
	if (pNode)
	{
		bool bVisible = bChildVisible;
		//设置所有子节点
		SetChildVisible(pNode, bVisible);
	}
}
     
    
bool CNewChannelListItemAdapter::OnGroupClick(EventArgs *e)
{
	SItemPanel *pItem = sobj_cast<SItemPanel>(e->sender); 
    int position = (int)pItem->GetItemIndex(); 
	Node<NewChannelInfo> *pNode = (Node<NewChannelInfo> *)pItem->GetUserData();
	bool bChildVisible = true;
	if (pNode && pNode->data().child_visible_)
	{
		bChildVisible = false;
	}

	ExpendNode(position, bChildVisible);

    notifyDataSetChanged();

    return true;
} 

bool CNewChannelListItemAdapter::OnItemDblClick(SOUI::EventArgs *e)
{
	SItemPanel *pItem = sobj_cast<SItemPanel>(e->sender);
	SASSERT(pItem);
	unsigned int position = pItem->GetItemIndex();
	
	Node<NewChannelInfo> *pNode = GetNode(position);
	
	if (pNode)
	{
		if(!pNode->data().folder_ && m_pCB)
			m_pCB->OnItemDBClick(m_cb_data, position, &pNode->data().value);
	} 

	return true;
}

}

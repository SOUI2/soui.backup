#include "stdafx.h"
#include "ui/adapter/HotChannelListItemAdapter.h"
#include "ui/SToggleEx.h"

#include "res/R.h" 
 
namespace SOUI
{  

const wchar_t * KHotChannelAttrName_Height[] = {
	L"defHeight",
	L"friendItemHeight" 
};

CHotChannelListItemAdapter::CHotChannelListItemAdapter(int data, live::IListItemCallback *pCB)
{ 
	this->SetCallback(data, pCB);

	root_node_ = new Node<HotChannelInfo>();
    root_node_->data().level_ = -1;
    root_node_->data().child_visible_ = true;
    root_node_->data().has_child_ = true; 
}

CHotChannelListItemAdapter::~CHotChannelListItemAdapter()
{
    Clear();
} 

void CHotChannelListItemAdapter::RemoveAll()
{ 
    for (int i = 0; i < root_node_->num_children(); ++i)
    {
		Node<HotChannelInfo>* child = root_node_->child(i);
        RemoveNode(child);
    }
    delete root_node_;

	root_node_ = new Node<HotChannelInfo>();
    root_node_->data().level_ = -1;
    root_node_->data().child_visible_ = true;
    root_node_->data().has_child_ = true; 
} 

Node<HotChannelInfo>* CHotChannelListItemAdapter::GetRoot()
{
    return root_node_;
}
 
Node<HotChannelInfo>* CHotChannelListItemAdapter::AddGroupNode(const std::wstring &title, bool has_child,Node<HotChannelInfo>* parent)
{
	if (!parent)
		parent = root_node_;


	Node<HotChannelInfo>* node = new Node<HotChannelInfo>();

	node->data().id_ = m_Nodes.getSeqence();
	node->data().level_ = parent->data().level_ + 1;
	node->data().has_child_ = has_child;
	 
	node->data().folder_ = true;
	 
	node->data().child_visible_ = true;

	node->data().text_ = title;
	node->data().value = NULL;

	parent->add_child(node);
	 
	m_Nodes.AddID(node->data().id_, node);
	return node;
}
 
Node<HotChannelInfo>* CHotChannelListItemAdapter::AddNode(const std::wstring &title, unsigned int  channel_id, unsigned int  owner_uid, bool has_child, Node<HotChannelInfo>* parent)
{
    if (!parent)
        parent = root_node_;  
	 
	HotChannelInfo *pUserData = new HotChannelInfo();
	pUserData->channel_id = channel_id;
	pUserData->owner_uid = owner_uid;

	Node<HotChannelInfo> *node = new Node<HotChannelInfo>;

	node->data().id_ = m_Nodes.getSeqence();
    node->data().level_ = parent->data().level_ + 1;
    node->data().has_child_ = has_child; 
	node->data().folder_ = has_child;
	 
    node->data().child_visible_ = false;

	node->data().text_ = title;
	node->data().value = pUserData;

    parent->add_child(node);

	m_Nodes.AddID(node->data().id_, node);
    return node;
}

bool CHotChannelListItemAdapter::RemoveNode(Node<HotChannelInfo>* node)
{
    if (!node || node == root_node_) return false;

    for (int i = 0; i < node->num_children(); ++i)
    {
		Node<HotChannelInfo>* child = node->child(i);
        RemoveNode(child);
    }
 
    node->parent()->remove_child(node);
    delete node;

    return true;
}

void CHotChannelListItemAdapter::SetChildVisible(Node<HotChannelInfo>* node, bool visible)
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
			Node<HotChannelInfo> *pNode = node->child(i);
			m_Nodes.SetVisible(prior_id, pNode->data().id_);

			prior_id = pNode->data().id_;
		}  
	}
	else
	{
		int child_count = node->num_children();

		for (int i = 0; i <child_count; ++i)
		{
			Node<HotChannelInfo> *pNode = node->child(i);
			m_Nodes.SetInVisible(node->data().id_, pNode->data().id_);
		} 
	}
}

bool CHotChannelListItemAdapter::CanExpand(Node<HotChannelInfo>* node) const
{
    if (!node || node == root_node_)
        return false;

    return node->data().has_child_;
}

void CHotChannelListItemAdapter::SetCallback(int data, live::IListItemCallback *pCB)
{
	m_cb_data = data;
	m_pCB = pCB;
}
    
void CHotChannelListItemAdapter::Clear()
{
	RemoveAll();

    if (root_node_)
        delete root_node_;

    root_node_ = NULL;

} 

void CHotChannelListItemAdapter::InitByTemplate(pugi::xml_node xmlTemplate)
{
	m_nItemHeight[0] = xmlTemplate.attribute(KHotChannelAttrName_Height[0]).as_int(30);
	m_nItemHeight[1] = xmlTemplate.attribute(KHotChannelAttrName_Height[1]).as_int(60); 
}
 
int CHotChannelListItemAdapter::EnsureKeyVisible(BOOL bExpired, unsigned int  id)
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
    
int CHotChannelListItemAdapter::getCount()
{
	return m_Nodes.GetCachelayerCount();
}

Node<HotChannelInfo> *CHotChannelListItemAdapter::GetNode(int id)
{
	return (Node<HotChannelInfo> *)m_Nodes.getNode(id);
}

int CHotChannelListItemAdapter::getItemViewType(int position)
{
	Node<HotChannelInfo> *pNode = GetNode(position);
	if (pNode && pNode->data().folder_)
        return VT_GROUP;
    else
        return VT_DATA;
}

SIZE CHotChannelListItemAdapter::getViewDesiredSize(int position, SWindow *pItem, LPCRECT prcContainer)
{ 
//	Node<HotChannelInfo> *pNode = GetNode(position);
	int viewType = getItemViewType(position);
	return CSize(0, m_nItemHeight[viewType]);//cx在listview，mclistview中没有使用，不需要计算
}
 
int CHotChannelListItemAdapter::getViewTypeCount()
{
    return 2;
}

SStringW CHotChannelListItemAdapter::GetColumnName(int iCol) const
{
    SStringW colNames[] = {L"col_nick"};
    return colNames[iCol];
}

void CHotChannelListItemAdapter::getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
{
	int nViewType = VT_GROUP;
	Node<HotChannelInfo> *pNode = GetNode(position);
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
		pItem->GetEventSet()->unsubscribeEvent(EVT_ITEMPANEL_CLICK, Subscriber(&CHotChannelListItemAdapter::OnGroupClick, this));
		pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_CLICK, Subscriber(&CHotChannelListItemAdapter::OnGroupClick, this));
		 
    }
	else
    {
		pItem->SetUserData((ULONG_PTR)pNode);
		if(pNode->data().child_visible_)
		{   
			pItem->FindChildByID(R.id.txt_name)->SetWindowText(pNode->data().text_.c_str());

			HotChannelInfo *pItemData = pNode->data().value;
			wchar_t sUID[128];
			swprintf(sUID, L"%d", pItemData->owner_uid);

			pItem->FindChildByID(R.id.txt_desc)->SetWindowText(sUID);

			pItem->GetEventSet()->unsubscribeEvent(EVT_ITEMPANEL_DBCLICK, Subscriber(&CHotChannelListItemAdapter::OnItemDblClick, this));
			pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_DBCLICK, Subscriber(&CHotChannelListItemAdapter::OnItemDblClick, this));
		}
		else
		{ 
			pItem->FindChildByID(R.id.txt_name)->SetWindowText(pNode->data().text_.c_str());			
			

			HotChannelInfo *pItemData = pNode->data().value;
			wchar_t sUID[128];
			swprintf(sUID, L"%d", pItemData->owner_uid);

			pItem->FindChildByID(R.id.txt_desc)->SetWindowText(sUID);

		}
    }
}

void CHotChannelListItemAdapter::ExpendNode(int position, bool bChildVisible)
{
	Node<HotChannelInfo> *pNode = GetNode(position);
	if (pNode)
	{
		bool bVisible = bChildVisible;
		//设置所有子节点
		SetChildVisible(pNode, bVisible);
	}
}
     
    
bool CHotChannelListItemAdapter::OnGroupClick(EventArgs *e)
{
	SItemPanel *pItem = sobj_cast<SItemPanel>(e->sender); 
    int position = (int)pItem->GetItemIndex(); 
	Node<HotChannelInfo> *pNode = (Node<HotChannelInfo> *)pItem->GetUserData();
	bool bChildVisible = true;
	if (pNode && pNode->data().child_visible_)
	{
		bChildVisible = false;
	}

	ExpendNode(position, bChildVisible);

    notifyDataSetChanged();

    return true;
} 

bool CHotChannelListItemAdapter::OnItemDblClick(SOUI::EventArgs *e)
{
	SItemPanel *pItem = sobj_cast<SItemPanel>(e->sender);
	SASSERT(pItem);
	unsigned int position = pItem->GetItemIndex();
	
	Node<HotChannelInfo> *pNode = GetNode(position);
	
	if (pNode)
	{
		if(!pNode->data().folder_ && m_pCB)
			m_pCB->OnItemDBClick(m_cb_data, position, &pNode->data().value);
	} 

	return true;
}

}

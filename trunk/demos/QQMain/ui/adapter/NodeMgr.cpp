#include "NodeMgr.h"
#include "ui/adapter/SimpleListDef.h"
#include <Windows.h>
#include <algorithm>

NodeMgr::NodeMgr()
{
	m_max_id_count = 0x9FFFF;
	ClearID();
}

int NodeMgr::getSeqence()
{
	if (m_id + 1<m_max_id_count)
		return ++m_id;
	return 0;
}

void NodeMgr::AddID(int id, void *pData)
{
	NodeSeq s;
	s.id = id;
	s.pData = pData;

	m_id_map.insert(std::map<int, NodeSeq >::value_type(id, s));
	m_cache_layer.push_back(id);
}

void NodeMgr::SetIDMaxCount(int max_id_count)
{
	m_max_id_count = max_id_count;
}

int NodeMgr::GetIDMaxCount()
{
	return m_max_id_count;
}

int NodeMgr::GetIDCount()
{
	return m_id_map.size();
}

int NodeMgr::GetCachelayerCount()
{
	return m_cache_layer.size();
}

int NodeMgr::RemoveID(int id)
{
	int real_id = 0;
	std::map<int, NodeSeq >::iterator itr = m_id_map.find(id);
	if (itr != m_id_map.end())
	{
		real_id = itr->second.id;
		m_id_map.erase(itr);
	}

	std::vector<int>::iterator itrcache = std::find(m_cache_layer.begin(), m_cache_layer.end(), id);
	if (itrcache != m_cache_layer.end())
	{ 
		m_cache_layer.erase(itrcache);
	} 

	return real_id;
}

void *NodeMgr::Node(int id)
{
	std::map<int, NodeSeq >::iterator itr = m_id_map.find(id);
	if (itr != m_id_map.end())
		return itr->second.pData;

	return NULL;
} 

int NodeMgr::SetVisible(int prior_id, int id)
{
	std::vector<int>::iterator itrcache = std::find(m_cache_layer.begin(), m_cache_layer.end(), prior_id);
	if (itrcache != m_cache_layer.end())
	{
		m_cache_layer.insert(itrcache+1,id);
	}

	return 0;
}

int NodeMgr::SetInVisible(int parent_id, int id)
{
	std::vector<int>::iterator itrcache = std::find(m_cache_layer.begin(), m_cache_layer.end(),id);
	if (itrcache != m_cache_layer.end())
	{
		m_cache_layer.erase(itrcache);
	}

	return 0;
}

void *NodeMgr::getNode(int id)
{
	if ((unsigned int)id >= m_cache_layer.size())
		return NULL;

	int real_id = m_cache_layer[id]; 
	return Node(real_id);	
}

void NodeMgr::ClearID()
{ 
	m_id = -1;
	m_id_map.clear();
}
 
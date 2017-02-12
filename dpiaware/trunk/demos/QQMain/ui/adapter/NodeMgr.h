#ifndef _NODE_MGR_H
#define _NODE_MGR_H

#include "ui/adapter/SimpleListDef.h" 
#include <map>
#include <vector>

class NodeMgr
{
    struct NodeSeq
    {
        int id;
		void *pData;
    };
private: 
    int m_id;
    int m_max_id_count;
	std::map<int, NodeSeq > m_id_map;
	std::vector<int> m_cache_layer; //加一个缓存层，简单处理隐藏显示
public:
    NodeMgr();
     
    int getSeqence();
	void AddID(int id, void *pData);
	int RemoveID(int id);
	int GetCachelayerCount();
	int SetInVisible(int parent_id,int id);
	int SetVisible(int prior_id, int id);
	void *Node(int id);
	void *getNode(int id);
    void ClearID();
    int GetIDCount();

    void SetIDMaxCount(int max_id_count);
    int GetIDMaxCount();
};
 

#endif //_NODE_MGR_H
#ifndef _SIMPLE_LIST_DEF_H_
#define _SIMPLE_LIST_DEF_H_

#include <vector>
#include <math.h>
#include <string>

namespace SOUI
{  

template <typename T>
class Node
{
public: 
	struct NodeData
	{
		NodeData()
		{
			id_ = -1;
			value = NULL;
		}

		int id_;
		int level_;
		bool folder_;
		bool child_visible_;
		bool has_child_;

		std::wstring text_;
		T *value;
	}; 
private:
	typedef std::vector <Node*>	Children;

	Children	children_;
	Node*		parent_;

	NodeData    data_; 
public:

	Node()
		: parent_(NULL)
	{

	}

	Node(NodeData t)
		: data_(t)
		, parent_(NULL)
	{

	}

	Node(NodeData t, Node* parent)
		: data_(t)
		, parent_(parent)
	{


	}

	~Node()
	{
		if (data_.value)
		{
			delete (data_.value);
		}

		data_.value = NULL;

		for (int i = 0; i < num_children(); ++i)
		{
			delete children_[i];
		}
	}


	void set_parent(Node* parent)
	{
		parent_ = parent;
	}


	NodeData& data()
	{
		return data_;
	}

	int num_children() const
	{
		return static_cast<int>(children_.size());
	}

	Node* child(int i)
	{
		return children_[i];
	}

	Node* parent()
	{
		return (parent_);
	}

	bool has_children() const
	{
		return num_children() > 0;
	}

	bool folder() const
	{
		return data_.folder_;
	}

	void add_child(Node* child)
	{
		child->set_parent(this);
		children_.push_back(child);
	}


	void remove_child(Node* child)
	{
		Children::iterator iter = children_.begin();
		for (; iter < children_.end(); ++iter)
		{
			if (*iter == child)
			{
				children_.erase(iter);
				return;
			}
		}
	}

	Node* get_last_child()
	{
		if (has_children())
		{
			return child(num_children() - 1)->get_last_child();
		}
		return this;
	}

};

} // SOUI

#endif // _SIMPLE_LIST_DEF_H_
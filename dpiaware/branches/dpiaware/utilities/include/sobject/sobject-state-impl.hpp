#pragma once
#include "../souicoll.h"
#include "sobject-state-i.h"

namespace SOUI
{
	template<class T>
	class SObjectStateImpl : public T
	{
	public:
		virtual void AddListener(IObjectStateListener * pListener)
		{
			for(size_t i=0;i<m_lstListener.GetCount();i++)
			{
				if(m_lstListener[i] == pListener) return;
			}
			m_lstListener.Add(pListener);
			pListener->AddRef();
		}

		virtual void RemoveListener(IObjectStateListener * pListener)
		{
			for(size_t i=0;i<m_lstListener.GetCount();i++)
			{
				if(m_lstListener[i] == pListener)
				{
					m_lstListener.RemoveAt(i);
					pListener->Release();
					break;
				}
			}
		}

		virtual void NotifyStateChanged()
		{
			for(size_t i=0;i<m_lstListener.GetCount();i++)
			{
				m_lstListener[i]->OnObjectStateChanged(static_cast<IObject *>(this));
			}
		}

	protected:
		SArray<IObjectStateListener *> m_lstListener;
	};
}
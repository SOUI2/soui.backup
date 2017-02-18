#pragma once

#include "../unknown/obj-ref-i.h"
#include "sobject-i.h"

namespace SOUI
{
	struct IObjectStateListener : public IObjRef{
		virtual void OnObjectStateChanged(const IObject * pObj) = 0;
	};

	struct IObjectState{
		virtual void AddListener(IObjectStateListener *pListener) = 0;

		virtual void RemoveListener(IObjectStateListener *pListener) = 0;

		virtual void NotifyStateChanged() = 0;
	};

}
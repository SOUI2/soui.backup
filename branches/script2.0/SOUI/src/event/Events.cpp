#include "souistd.h"

#include "core/SWnd.h"
#include "core/SItempanel.h"
#include "event/events.h"

namespace SOUI
{
    EventArgs::EventArgs( SObject *pSender ) : handled(0)
        , sender(pSender)
    {
        idFrom = pSender->GetID();
        nameFrom = pSender->GetName();
    }


    EventOfPanel::EventOfPanel( SItemPanel *_pPanel,EventArgs *_pOrgEvt )
        :EventArgs(_pPanel)
        ,pPanel(_pPanel)
        ,pOrgEvt(_pOrgEvt)
    {

    }

    EventOfComoboxExItem::EventOfComoboxExItem(SObject *pSender,EventCmd *_pOrgEvt )
        :EventArgs(pSender)
        ,pOrgEvt(_pOrgEvt)
        ,bCancel(FALSE)
    {

    }

}
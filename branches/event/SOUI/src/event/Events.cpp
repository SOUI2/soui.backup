#include "duistd.h"

#include "duiwnd.h"
#include "duiItempanel.h"
#include "event/events.h"

namespace SOUI
{
    EventArgs::EventArgs( SWindow *pSender ) : handled(0)
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

}
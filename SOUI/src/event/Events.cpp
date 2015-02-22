#include "souistd.h"
#include "event/events.h"
#include "core/SItempanel.h"

namespace SOUI
{
    EventOfPanel::EventOfPanel( SItemPanel *_pPanel,EventArgs *_pOrgEvt )
        :TplEventArgs<EventOfPanel>(_pPanel)
        ,pPanel(_pPanel)
        ,pOrgEvt(_pOrgEvt)
    {

    }
}
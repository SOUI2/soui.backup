#pragma once

namespace SOUI
{
    class SOUI_EXP CDuiRef
    {
    public:
        CDuiRef():m_nRef(1)
        {
        }

        ~CDuiRef()
        {

        }

        int AddRef()
        {
            return ++m_nRef;
        }

        int Release()
        {
            int nRet=--m_nRef;
            if(nRet==0)
            {
                OnFinalRelease();
            }
            return nRet;
        }

    protected:
        virtual void OnFinalRelease()=NULL;

        int m_nRef;
    };

}//namespace SOUI
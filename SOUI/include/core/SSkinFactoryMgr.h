#pragma once
#include "interface/sskinobj-i.h"

namespace SOUI
{

    class SSkinFactory
    {
    public:
        virtual ~SSkinFactory() {}
        virtual ISkinObj * NewSkin()=NULL;
        virtual const SStringW & GetTypeName()=NULL;
        virtual SSkinFactory * Clone()=NULL;
    };

    template<typename T>
    class TplSkinFactory :public SSkinFactory
    {
    public:
        TplSkinFactory():m_strTypeName(T::GetClassName())
        {
        }

        virtual ISkinObj * NewSkin()
        {
            return new T;
        }

        virtual const SStringW & GetTypeName()
        {
            return m_strTypeName;
        }

        virtual SSkinFactory * Clone()
        {
            return new TplSkinFactory<T>;
        }
    protected:
        SStringW m_strTypeName;
    };

    typedef SSkinFactory * SSkinFactoryPtr;
    class SOUI_EXP SSkinFactoryMgr: public SCmnMap<SSkinFactoryPtr,SStringW>
    {
    public:
        SSkinFactoryMgr()
        {
            m_pFunOnKeyRemoved=OnSkinRemoved;
            AddStandardSkin();
        }

        bool RegisterSkinFactory(SSkinFactory &skinFactory,bool bReplace=false)
        {
            if(HasKey(skinFactory.GetTypeName()))
            {
                if(!bReplace) return false;
                RemoveKeyObject(skinFactory.GetTypeName());
            }
            AddKeyObject(skinFactory.GetTypeName(),skinFactory.Clone());
            return true;
        }

        bool UnregisterSkinFactory(const SStringW & strClassName)
        {
            return RemoveKeyObject(strClassName);
        }

        ISkinObj * CreateSkinByName(LPCWSTR pszClassName);

    protected:
        static void OnSkinRemoved(const SSkinFactoryPtr & obj);
        void AddStandardSkin();
    };
}//end of namespace
#pragma once
#include "interface/sskinobj-i.h"

namespace SOUI
{

    class CSkinFactory
    {
    public:
        virtual ~CSkinFactory() {}
        virtual ISkinObj * NewSkin()=NULL;
        virtual const SStringW & GetTypeName()=NULL;
        virtual CSkinFactory * Clone()=NULL;
    };

    template<typename T>
    class TplSkinFactory :public CSkinFactory
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

        virtual CSkinFactory * Clone()
        {
            return new TplSkinFactory<T>;
        }
    protected:
        SStringW m_strTypeName;
    };

    typedef CSkinFactory * CSkinFactoryPtr;
    class SOUI_EXP SSkinFactoryMgr: public SCmnMap<CSkinFactoryPtr,SStringW>
    {
    public:
        SSkinFactoryMgr()
        {
            m_pFunOnKeyRemoved=OnSkinRemoved;
            AddStandardSkin();
        }

        bool RegisterSkinFactory(CSkinFactory &skinFactory,bool bReplace=false)
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
        static void OnSkinRemoved(const CSkinFactoryPtr & obj);
        void AddStandardSkin();
    };
}//end of namespace
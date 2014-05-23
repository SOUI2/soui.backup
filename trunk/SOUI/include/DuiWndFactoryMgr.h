#pragma once
#include "duisingletonmap.h"


namespace SOUI
{
    class CDuiWindow;

class CDuiWindowFactory
{
public:
    virtual ~CDuiWindowFactory() {}
    virtual CDuiWindow* NewWindow() = 0;
    virtual LPCSTR DuiWindowBaseName()=0;

    virtual const CDuiStringA & getWindowType()=0;

    virtual CDuiWindowFactory* Clone() const =0;
};

template <typename T>
class TplDuiWindowFactory : public CDuiWindowFactory
{
public:
    //! Default constructor.
    TplDuiWindowFactory():m_strTypeName(T::GetClassName())
    {
    }

    LPCSTR DuiWindowName(){return T::GetClassName();}

    LPCSTR DuiWindowBaseName(){return T::BaseClassName();}

    // Implement WindowFactory interface
    CDuiWindow* NewWindow()
    {
        return new T;
    }

    virtual const CDuiStringA & getWindowType()
    {
        return m_strTypeName;
    }

    virtual CDuiWindowFactory* Clone() const 
    {
        return new TplDuiWindowFactory();
    }
protected:
    CDuiStringA m_strTypeName;
};



typedef CDuiWindowFactory* CDuiWindowFactoryPtr;
class SOUI_EXP DuiWindowFactoryMgr :
    public DuiCmnMap<CDuiWindowFactoryPtr,CDuiStringA>
{
public:
    DuiWindowFactoryMgr(void);

    //************************************
    // Method:    RegisterFactory,注册APP自定义的窗口类
    // FullName:  SOUI::DuiWindowFactoryManager::RegisterFactory
    // Access:    public
    // Returns:   bool
    // Qualifier:
    // Parameter: CDuiWindowFactory * pWndFactory:窗口工厂指针
    // Parameter: bool bReplace:强制替换原有工厂标志
    //************************************
    bool RegisterWndFactory(CDuiWindowFactory & wndFactory,bool bReplace=false)
    {
        if(HasKey(wndFactory.getWindowType()))
        {
            if(!bReplace) return false;
            RemoveKeyObject(wndFactory.getWindowType());
        }
        AddKeyObject(wndFactory.getWindowType(),wndFactory.Clone());
        return true;
    }

    //************************************
    // Method:    UnregisterFactor,反注册APP自定义的窗口类
    // FullName:  SOUI::DuiWindowFactoryManager::UnregisterFactor
    // Access:    public
    // Returns:   bool
    // Qualifier:
    // Parameter: CDuiWindowFactory * pWndFactory
    //************************************
    bool UnregisterWndFactory(const CDuiStringA & strClassType)
    {
        return  RemoveKeyObject(strClassType);
    }

    CDuiWindow *CreateWindowByName(LPCSTR pszClassName);

    LPCSTR BaseClassNameFromClassName(LPCSTR pszClassName);
protected:
    static void OnWndFactoryRemoved(const CDuiWindowFactoryPtr & obj);

    void AddStandardWindowFactory();
};

}//namespace SOUI
#pragma once

#include "Events.h"

// Start of SOUI namespace section
namespace SOUI
{

enum _SLOTTYPE{SLOT_FUN,SLOT_MEMBER,SLOT_USER};
/*!
\brief
    Defines abstract interface which will be used when constructing various
    functor objects that bind slots to signals (or in CEGUI terms, handlers to
    events).
*/
struct SOUI_EXP ISlotFunctor
{
    virtual ~ISlotFunctor() {};
    virtual bool operator()(EventArgs *pArg) = 0;
    virtual ISlotFunctor* Clone() const =0;
    virtual bool Equal(const ISlotFunctor & sour)const  =0;
    virtual UINT GetSlotType() const  =0;
};

/*!
\brief
    Slot functor class that calls back via a free function pointer.
*/
class SOUI_EXP FreeFunctionSlot : public ISlotFunctor
{
public:
    //! Slot function type.
    typedef bool (SlotFunction)(EventArgs *);

    FreeFunctionSlot(SlotFunction* func) :
        d_function(func)
    {}

    virtual bool operator()(EventArgs *pArg)
    {
        return d_function(pArg);
    }

    virtual ISlotFunctor* Clone() const 
    {
        return new FreeFunctionSlot(d_function);
    }

    virtual bool Equal(const ISlotFunctor & sour)const 
    {
        if(sour.GetSlotType()!=SLOT_FUN) return false;
        const FreeFunctionSlot *psour=static_cast<const FreeFunctionSlot*>(&sour);
        SASSERT(psour);
        return psour->d_function==d_function;
    }

    virtual UINT GetSlotType() const {return SLOT_FUN;}

private:
    SlotFunction* d_function;
};

/*!
\brief
    Slot template class that creates a functor that calls back via a class
    member function.
*/
template<typename T>
class MemberFunctionSlot : public ISlotFunctor
{
public:
    //! Member function slot type.
    typedef bool(T::*MemberFunctionType)(EventArgs *);

    MemberFunctionSlot(MemberFunctionType func, T* obj) :
        d_function(func),
        d_object(obj)
    {}

    virtual bool operator()(EventArgs *pArg)
    {
        return (d_object->*d_function)(pArg);
    }

    virtual ISlotFunctor* Clone() const 
    {
        return new MemberFunctionSlot(d_function,d_object);
    }

    virtual bool Equal(const ISlotFunctor & sour)const 
    {
        if(sour.GetSlotType()!=SLOT_MEMBER) return false;
        const MemberFunctionSlot *psour=static_cast<const MemberFunctionSlot*>(&sour);
        SASSERT(psour);
        return psour->d_function==d_function && psour->d_object==d_object;
    }

    virtual UINT GetSlotType()const {return SLOT_MEMBER;}

private:
    MemberFunctionType d_function;
    T* d_object;
};

template <class T>
MemberFunctionSlot<T> Subscriber( bool (T::* pFn)(EventArgs *), T* pObject)
{
    return MemberFunctionSlot<T>(pFn,pObject);
}

inline FreeFunctionSlot Subscriber(bool (*pFn)(EventArgs *))
{
    return FreeFunctionSlot(pFn); 
}

} // End of  CEGUI namespace section
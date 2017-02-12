// translator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "uianimation.h"
#include <search.h>

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    // SUIAnimationVariable
    SUIAnimationVariable::SUIAnimationVariable()
    :m_uTag(0)
    ,m_value(0.0)
    ,m_prevValue(0.0)
    ,m_min(0.0)
    ,m_max(0.0)
    ,m_final(0.0)
    {
        
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::GetValue(/* [annotation][retval][out] */ DOUBLE *value)
    {
        *value = m_value;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::GetFinalValue(/* [annotation][retval][out] */ DOUBLE *finalValue)
    {
        *finalValue = m_final;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::GetPreviousValue(/* [annotation][retval][out] */ DOUBLE *previousValue)
    {
        *previousValue = m_prevValue;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::GetCurrentStoryboard(/* [annotation][retval][out] */ IUIAnimationStoryboard **storyboard)
    {
        if(!m_storyboard)
        {
            return E_FAIL;
        }
        *storyboard = m_storyboard;
        (*storyboard)->AddRef();
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::SetLowerBound(/* [annotation][in] */ DOUBLE bound)
    {
        m_min = bound;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::SetUpperBound(/* [annotation][in] */ DOUBLE bound)
    {
        m_max = bound;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::SetTag(/* [annotation][in] */ UINT32 id)
    {
        m_uTag = id;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::GetTag(/* [annotation][out] */ UINT32 *id)
    {
        *id = m_uTag;
        return S_OK;
    }

    /* [annotation] */ 
    HRESULT SUIAnimationVariable::SetVariableChangeHandler(/* [annotation][unique][in] */ IUIAnimationVariableChangeHandler *handler)
    {
        m_valueChangeHandler = handler;
        return S_OK;
    }
    
    
    //////////////////////////////////////////////////////////////////////////

}

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TRANSLATOR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TRANSLATOR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#include <souicoll.h>
#include <unknown/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <interface/uianimation-i.h>

namespace SOUI
{
    class SUIAnimationVariable : public TObjRefImpl<IUIAnimationVariable>
    {
    public:
        SUIAnimationVariable();
    
        virtual /* [annotation] */ 
            HRESULT  GetValue( 
            /* [annotation][retval][out] */ 
            DOUBLE *value);

        virtual /* [annotation] */ 
            HRESULT  GetFinalValue( 
            /* [annotation][retval][out] */ 
            DOUBLE *finalValue);

        virtual /* [annotation] */ 
            HRESULT  GetPreviousValue( 
            /* [annotation][retval][out] */ 
            DOUBLE *previousValue);

        virtual /* [annotation] */ 
            HRESULT  GetCurrentStoryboard( 
            /* [annotation][retval][out] */ 
            IUIAnimationStoryboard **storyboard);

        virtual /* [annotation] */ 
            HRESULT  SetLowerBound( 
            /* [annotation][in] */ 
            DOUBLE bound);

        virtual /* [annotation] */ 
            HRESULT  SetUpperBound( 
            /* [annotation][in] */ 
            DOUBLE bound);

        virtual /* [annotation] */ 
            HRESULT  SetTag( 
            /* [annotation][in] */ 
            UINT32 id);

        virtual /* [annotation] */ 
            HRESULT  GetTag( 
            /* [annotation][out] */ 
            UINT32 *id);

        virtual /* [annotation] */ 
            HRESULT  SetVariableChangeHandler( 
            /* [annotation][unique][in] */ 
            IUIAnimationVariableChangeHandler *handler);

    protected:
        CAutoRefPtr<IUIAnimationVariableChangeHandler>  m_valueChangeHandler;
        CAutoRefPtr<IUIAnimationStoryboard>             m_storyboard;
        UINT32  m_uTag;
        DOUBLE  m_prevValue;
        DOUBLE  m_value;
        DOUBLE  m_min;
        DOUBLE  m_max;
        DOUBLE  m_final;
    };
    
    
    class SUIAnimationStoryboard : public TObjRefImpl<IUIAnimationStoryboard>
    {
        struct VarTransPair
        {
            CAutoRefPtr<IUIAnimationVariable> var;
            CAutoRefPtr<IUIAnimationTransition> trans;
        };
        
        struct KEYFRAMEINFO
        {
        UI_ANIMATION_KEYFRAME keyFrame;         //key frame
        UINT32                repetitionCount;
        SList<VarTransPair>   lstVarTransPair;
        };
        
    public:
    
        virtual /* [annotation] */ 
            HRESULT  AddTransition( 
            /* [annotation][in] */ 
            IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            IUIAnimationTransition *transition);

        virtual /* [annotation] */ 
            HRESULT  AddKeyframeAtOffset( 
            /* [annotation][in] */ 
            UI_ANIMATION_KEYFRAME existingKeyframe,
            /* [annotation][in] */ 
            UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            UI_ANIMATION_KEYFRAME *keyframe);

        virtual /* [annotation] */ 
            HRESULT  AddKeyframeAfterTransition( 
            /* [annotation][in] */ 
            IUIAnimationTransition *transition,
            /* [annotation][retval][out] */ 
            UI_ANIMATION_KEYFRAME *keyframe);

        virtual /* [annotation] */ 
            HRESULT  AddTransitionAtKeyframe( 
            /* [annotation][in] */ 
            IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            UI_ANIMATION_KEYFRAME startKeyframe);

        virtual /* [annotation] */ 
            HRESULT  AddTransitionBetweenKeyframes( 
            /* [annotation][in] */ 
            IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            UI_ANIMATION_KEYFRAME startKeyframe,
            /* [annotation][in] */ 
            UI_ANIMATION_KEYFRAME endKeyframe);

        virtual /* [annotation] */ 
            HRESULT  RepeatBetweenKeyframes( 
            /* [annotation][in] */ 
            UI_ANIMATION_KEYFRAME startKeyframe,
            /* [annotation][in] */ 
            UI_ANIMATION_KEYFRAME endKeyframe,
            /* [annotation][in] */ 
            INT32 repetitionCount);

        virtual /* [annotation] */ 
            HRESULT  Schedule( 
            /* [annotation][in] */ 
            UI_ANIMATION_SECONDS timeNow,
            /* [annotation][defaultvalue][out] */ 
            UI_ANIMATION_SCHEDULING_RESULT *schedulingResult);

        virtual /* [annotation] */ 
            HRESULT  Conclude( void);

        virtual /* [annotation] */ 
            HRESULT  Finish( 
            /* [annotation][in] */ 
            UI_ANIMATION_SECONDS completionDeadline);

        virtual /* [annotation] */ 
            HRESULT  Abandon( void);

        virtual /* [annotation] */ 
            HRESULT  SetTag( 
            /* [annotation][in] */ 
            UINT32 id);

        virtual /* [annotation] */ 
            HRESULT  GetTag( 
            /* [annotation][out] */ 
            UINT32 *id);

        virtual /* [annotation] */ 
            HRESULT  GetStatus( 
            /* [annotation][retval][out] */ 
            UI_ANIMATION_STORYBOARD_STATUS *status);

        virtual /* [annotation] */ 
            HRESULT  GetElapsedTime( 
            /* [annotation][out] */ 
            UI_ANIMATION_SECONDS *elapsedTime);

        virtual /* [annotation] */ 
            HRESULT  SetStoryboardEventHandler( 
            /* [annotation][unique][in] */ 
            IUIAnimationStoryboardEventHandler *handler);
            
    protected:
        CAutoRefPtr<IUIAnimationStoryboardEventHandler> m_evtHandler;    
        UINT                            m_uID;
        UI_ANIMATION_STORYBOARD_STATUS  m_status;
        UI_ANIMATION_SECONDS            m_tmElapsed;
        
        SList<KEYFRAMEINFO>             m_lstKeyFrameInfo;
    };
}
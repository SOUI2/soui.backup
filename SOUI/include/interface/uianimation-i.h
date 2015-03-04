
/* this ALWAYS GENERATED file contains the definitions for the interfaces */
 /* File created by MIDL compiler version 7.00.0555 */
/* Compiler settings for UIAnimation.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), struct */
/* @@MIDL_FILE_HEADING(  ) */
#pragma once
#include <wtypes.h>
#include <unknown/obj-ref-i.h>
namespace SOUI{
typedef DOUBLE UI_ANIMATION_SECONDS;
#define	UI_ANIMATION_SECONDS_EVENTUALLY	( -1 )
typedef /* [public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0000_0001
    {	UI_ANIMATION_UPDATE_NO_CHANGE	= 0,
	UI_ANIMATION_UPDATE_VARIABLES_CHANGED	= 1
    } 	UI_ANIMATION_UPDATE_RESULT;
    
typedef /* [public][public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0000_0002
    {	UI_ANIMATION_MANAGER_IDLE	= 0,
	UI_ANIMATION_MANAGER_BUSY	= 1
    } 	UI_ANIMATION_MANAGER_STATUS;
    
typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0000_0003
    {	UI_ANIMATION_MODE_DISABLED	= 0,
	UI_ANIMATION_MODE_SYSTEM_DEFAULT	= 1,
	UI_ANIMATION_MODE_ENABLED	= 2
    } 	UI_ANIMATION_MODE;
    
    struct IUIAnimationVariable;
    struct IUIAnimationVariable;
    struct IUIAnimationStoryboard;
    struct IUIAnimationVariable;
    struct IUIAnimationStoryboard;
    struct IUIAnimationManagerEventHandler;
    struct IUIAnimationPriorityComparison;
    struct IUIAnimationPriorityComparison;
    struct IUIAnimationPriorityComparison;
    struct IUIAnimationPriorityComparison;
    struct IUIAnimationStoryboard;
    struct IUIAnimationVariableChangeHandler;
    struct IUIAnimationVariableIntegerChangeHandler;
    struct IUIAnimationTransition;
    struct IUIAnimationTransition;
    struct IUIAnimationTransition;
    struct IUIAnimationTransition;
    struct IUIAnimationStoryboardEventHandler;
    struct IUIAnimationTimerUpdateHandler;
    struct IUIAnimationTimerEventHandler;
    struct IUIAnimationTimerClientEventHandler;
    
    //////////////////////////////////////////////////////////////////////////
    struct     IUIAnimationManager : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  CreateAnimationVariable( 
        /* [annotation][in] */ 
        DOUBLE initialValue,
        /* [annotation][retval][out] */ 
        IUIAnimationVariable **variable) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  ScheduleTransition( 
        /* [annotation][in] */ 
        IUIAnimationVariable *variable,
        /* [annotation][in] */ 
        IUIAnimationTransition *transition,
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS timeNow) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateStoryboard( 
        /* [annotation][retval][out] */ 
        IUIAnimationStoryboard **storyboard) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  FinishAllStoryboards( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS completionDeadline) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  AbandonAllStoryboards( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Update( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS timeNow,
        /* [annotation][defaultvalue][out] */ 
        UI_ANIMATION_UPDATE_RESULT *updateResult = 0) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetVariableFromTag( 
        /* [annotation][unique][in] */ 
        IObjRef *object,
        /* [annotation][in] */ 
        UINT32 id,
        /* [annotation][retval][out] */ 
        IUIAnimationVariable **variable) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetStoryboardFromTag( 
        /* [annotation][unique][in] */ 
        IObjRef *object,
        /* [annotation][in] */ 
        UINT32 id,
        /* [annotation][retval][out] */ 
        IUIAnimationStoryboard **storyboard) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetStatus( 
        /* [annotation][retval][out] */ 
        UI_ANIMATION_MANAGER_STATUS *status) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetAnimationMode( 
        /* [annotation][in] */ 
        UI_ANIMATION_MODE mode) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Pause( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Resume( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetManagerEventHandler( 
        /* [annotation][unique][in] */ 
        IUIAnimationManagerEventHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetCancelPriorityComparison( 
        /* [annotation][unique][in] */ 
        IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetTrimPriorityComparison( 
        /* [annotation][unique][in] */ 
        IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetCompressPriorityComparison( 
        /* [annotation][unique][in] */ 
        IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetConcludePriorityComparison( 
        /* [annotation][unique][in] */ 
        IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetDefaultLongestAcceptableDelay( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS delay) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Shutdown( void) = 0;
        
    };
    
typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0001_0001
    {	UI_ANIMATION_ROUNDING_NEAREST	= 0,
	UI_ANIMATION_ROUNDING_FLOOR	= 1,
	UI_ANIMATION_ROUNDING_CEILING	= 2
    } 	UI_ANIMATION_ROUNDING_MODE;
/* interface IUIAnimationVariable */
/* [unique][helpstring][uuid][object][local] */ 
    
    struct     IUIAnimationVariable : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  GetValue( 
        /* [annotation][retval][out] */ 
        DOUBLE *value) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetFinalValue( 
        /* [annotation][retval][out] */ 
        DOUBLE *finalValue) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetPreviousValue( 
        /* [annotation][retval][out] */ 
        DOUBLE *previousValue) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetIntegerValue( 
        /* [annotation][retval][out] */ 
        INT32 *value) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetFinalIntegerValue( 
        /* [annotation][retval][out] */ 
        INT32 *finalValue) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetPreviousIntegerValue( 
        /* [annotation][retval][out] */ 
        INT32 *previousValue) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetCurrentStoryboard( 
        /* [annotation][retval][out] */ 
        IUIAnimationStoryboard **storyboard) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetLowerBound( 
        /* [annotation][in] */ 
        DOUBLE bound) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetUpperBound( 
        /* [annotation][in] */ 
        DOUBLE bound) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetRoundingMode( 
        /* [annotation][in] */ 
        UI_ANIMATION_ROUNDING_MODE mode) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetTag( 
        /* [annotation][in] */ 
        UINT32 id) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetTag( 
        /* [annotation][out] */ 
        UINT32 *id) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetVariableChangeHandler( 
        /* [annotation][unique][in] */ 
        IUIAnimationVariableChangeHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetVariableIntegerChangeHandler( 
        /* [annotation][unique][in] */ 
        IUIAnimationVariableIntegerChangeHandler *handler) = 0;
        
    };
    
/* interface __MIDL_itf_UIAnimation_0000_0002 */
/* [local] */ 
typedef /* [public][public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0002_0001
    {	UI_ANIMATION_STORYBOARD_BUILDING	= 0,
	UI_ANIMATION_STORYBOARD_SCHEDULED	= 1,
	UI_ANIMATION_STORYBOARD_CANCELLED	= 2,
	UI_ANIMATION_STORYBOARD_PLAYING	= 3,
	UI_ANIMATION_STORYBOARD_TRUNCATED	= 4,
	UI_ANIMATION_STORYBOARD_FINISHED	= 5,
	UI_ANIMATION_STORYBOARD_READY	= 6,
	UI_ANIMATION_STORYBOARD_INSUFFICIENT_PRIORITY	= 7
    } 	UI_ANIMATION_STORYBOARD_STATUS;
    
typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0002_0002
    {	UI_ANIMATION_SCHEDULING_UNEXPECTED_FAILURE	= 0,
	UI_ANIMATION_SCHEDULING_INSUFFICIENT_PRIORITY	= 1,
	UI_ANIMATION_SCHEDULING_ALREADY_SCHEDULED	= 2,
	UI_ANIMATION_SCHEDULING_SUCCEEDED	= 3,
	UI_ANIMATION_SCHEDULING_DEFERRED	= 4
    } 	UI_ANIMATION_SCHEDULING_RESULT;
    
typedef struct __MIDL___MIDL_itf_UIAnimation_0000_0002_0003
    {
    int _;
    } 	*UI_ANIMATION_KEYFRAME;
    
#define	UI_ANIMATION_KEYFRAME_STORYBOARD_START	( ( UI_ANIMATION_KEYFRAME  )-1 )
#define	UI_ANIMATION_REPEAT_INDEFINITELY	( -1 )
    struct     IUIAnimationStoryboard : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  AddTransition( 
        /* [annotation][in] */ 
        IUIAnimationVariable *variable,
        /* [annotation][in] */ 
        IUIAnimationTransition *transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  AddKeyframeAtOffset( 
        /* [annotation][in] */ 
        UI_ANIMATION_KEYFRAME existingKeyframe,
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS offset,
        /* [annotation][retval][out] */ 
        UI_ANIMATION_KEYFRAME *keyframe) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  AddKeyframeAfterTransition( 
        /* [annotation][in] */ 
        IUIAnimationTransition *transition,
        /* [annotation][retval][out] */ 
        UI_ANIMATION_KEYFRAME *keyframe) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  AddTransitionAtKeyframe( 
        /* [annotation][in] */ 
        IUIAnimationVariable *variable,
        /* [annotation][in] */ 
        IUIAnimationTransition *transition,
        /* [annotation][in] */ 
        UI_ANIMATION_KEYFRAME startKeyframe) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  AddTransitionBetweenKeyframes( 
        /* [annotation][in] */ 
        IUIAnimationVariable *variable,
        /* [annotation][in] */ 
        IUIAnimationTransition *transition,
        /* [annotation][in] */ 
        UI_ANIMATION_KEYFRAME startKeyframe,
        /* [annotation][in] */ 
        UI_ANIMATION_KEYFRAME endKeyframe) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  RepeatBetweenKeyframes( 
        /* [annotation][in] */ 
        UI_ANIMATION_KEYFRAME startKeyframe,
        /* [annotation][in] */ 
        UI_ANIMATION_KEYFRAME endKeyframe,
        /* [annotation][in] */ 
        INT32 repetitionCount) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  HoldVariable( 
        /* [annotation][in] */ 
        IUIAnimationVariable *variable) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetLongestAcceptableDelay( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS delay) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Schedule( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS timeNow,
        /* [annotation][defaultvalue][out] */ 
        UI_ANIMATION_SCHEDULING_RESULT *schedulingResult = 0) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Conclude( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Finish( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS completionDeadline) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Abandon( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetTag( 
        /* [annotation][in] */ 
        UINT32 id) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetTag( 
        /* [annotation][out] */ 
        UINT32 *id) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetStatus( 
        /* [annotation][retval][out] */ 
        UI_ANIMATION_STORYBOARD_STATUS *status) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetElapsedTime( 
        /* [annotation][out] */ 
        UI_ANIMATION_SECONDS *elapsedTime) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetStoryboardEventHandler( 
        /* [annotation][unique][in] */ 
        IUIAnimationStoryboardEventHandler *handler) = 0;
        
    };
    struct     IUIAnimationTransition : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  SetInitialValue( 
        /* [annotation][in] */ 
        DOUBLE value) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetInitialVelocity( 
        /* [annotation][in] */ 
        DOUBLE velocity) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  IsDurationKnown( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetDuration( 
        /* [annotation][retval][out] */ 
        UI_ANIMATION_SECONDS *duration) = 0;
        
    };
    
    struct     IUIAnimationManagerEventHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnManagerStatusChanged( 
        /* [annotation][in] */ 
        UI_ANIMATION_MANAGER_STATUS newStatus,
        /* [annotation][in] */ 
        UI_ANIMATION_MANAGER_STATUS previousStatus) = 0;
        
    };
    
    struct     IUIAnimationVariableChangeHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnValueChanged( 
        /* [annotation][in] */ 
        IUIAnimationStoryboard *storyboard,
        /* [annotation][in] */ 
        IUIAnimationVariable *variable,
        /* [annotation][in] */ 
        DOUBLE newValue,
        /* [annotation][in] */ 
        DOUBLE previousValue) = 0;
        
    };
    
    struct     IUIAnimationVariableIntegerChangeHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnIntegerValueChanged( 
        /* [annotation][in] */ 
        IUIAnimationStoryboard *storyboard,
        /* [annotation][in] */ 
        IUIAnimationVariable *variable,
        /* [annotation][in] */ 
        INT32 newValue,
        /* [annotation][in] */ 
        INT32 previousValue) = 0;
        
    };
    
    struct     IUIAnimationStoryboardEventHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnStoryboardStatusChanged( 
        /* [annotation][in] */ 
        IUIAnimationStoryboard *storyboard,
        /* [annotation][in] */ 
        UI_ANIMATION_STORYBOARD_STATUS newStatus,
        /* [annotation][in] */ 
        UI_ANIMATION_STORYBOARD_STATUS previousStatus) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  OnStoryboardUpdated( 
        /* [annotation][in] */ 
        IUIAnimationStoryboard *storyboard) = 0;
        
    };
/* interface __MIDL_itf_UIAnimation_0000_0008 */
/* [local] */ 
typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0008_0001
    {	UI_ANIMATION_PRIORITY_EFFECT_FAILURE	= 0,
	UI_ANIMATION_PRIORITY_EFFECT_DELAY	= 1
    } 	UI_ANIMATION_PRIORITY_EFFECT;
/* interface IUIAnimationPriorityComparison */
/* [unique][helpstring][uuid][object][local] */ 
    struct     IUIAnimationPriorityComparison : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  HasPriority( 
        /* [annotation][in] */ 
        IUIAnimationStoryboard *scheduledStoryboard,
        /* [annotation][in] */ 
        IUIAnimationStoryboard *newStoryboard,
        /* [annotation][in] */ 
        UI_ANIMATION_PRIORITY_EFFECT priorityEffect) = 0;
        
    };
/* interface __MIDL_itf_UIAnimation_0000_0009 */
/* [local] */ 
typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0009_0001
    {	UI_ANIMATION_SLOPE_INCREASING	= 0,
	UI_ANIMATION_SLOPE_DECREASING	= 1
    } 	UI_ANIMATION_SLOPE;
    
    struct     IUIAnimationTransitionLibrary : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  CreateInstantaneousTransition( 
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateConstantTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateDiscreteTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS delay,
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS hold,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateLinearTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateLinearTransitionFromSpeed( 
        /* [annotation][in] */ 
        DOUBLE speed,
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateSinusoidalTransitionFromVelocity( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS period,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateSinusoidalTransitionFromRange( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][in] */ 
        DOUBLE minimumValue,
        /* [annotation][in] */ 
        DOUBLE maximumValue,
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS period,
        /* [annotation][in] */ 
        UI_ANIMATION_SLOPE slope,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateAccelerateDecelerateTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][in] */ 
        DOUBLE accelerationRatio,
        /* [annotation][in] */ 
        DOUBLE decelerationRatio,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateReversalTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateCubicTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration,
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][in] */ 
        DOUBLE finalVelocity,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateSmoothStopTransition( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS maximumDuration,
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  CreateParabolicTransitionFromAcceleration( 
        /* [annotation][in] */ 
        DOUBLE finalValue,
        /* [annotation][in] */ 
        DOUBLE finalVelocity,
        /* [annotation][in] */ 
        DOUBLE acceleration,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
    };
    
typedef /* [public][public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0010_0001
    {	UI_ANIMATION_DEPENDENCY_NONE	= 0,
        UI_ANIMATION_DEPENDENCY_INTERMEDIATE_VALUES	= 0x1,
        UI_ANIMATION_DEPENDENCY_FINAL_VALUE	= 0x2,
        UI_ANIMATION_DEPENDENCY_FINAL_VELOCITY	= 0x4,
        UI_ANIMATION_DEPENDENCY_DURATION	= 0x8
    } 	UI_ANIMATION_DEPENDENCIES;
    
    struct     IUIAnimationInterpolator : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  SetInitialValueAndVelocity( 
        /* [annotation][in] */ 
        DOUBLE initialValue,
        /* [annotation][in] */ 
        DOUBLE initialVelocity) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetDuration( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS duration) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetDuration( 
        /* [annotation][retval][out] */ 
        UI_ANIMATION_SECONDS *duration) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetFinalValue( 
        /* [annotation][retval][out] */ 
        DOUBLE *value) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  InterpolateValue( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS offset,
        /* [annotation][retval][out] */ 
        DOUBLE *value) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  InterpolateVelocity( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS offset,
        /* [annotation][retval][out] */ 
        DOUBLE *velocity) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetDependencies( 
        /* [annotation][out] */ 
        UI_ANIMATION_DEPENDENCIES *initialValueDependencies,
        /* [annotation][out] */ 
        UI_ANIMATION_DEPENDENCIES *initialVelocityDependencies,
        /* [annotation][out] */ 
        UI_ANIMATION_DEPENDENCIES *durationDependencies) = 0;
        
    };
    
    
    struct     IUIAnimationTransitionFactory : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  CreateTransition( 
        /* [annotation][in] */ 
        IUIAnimationInterpolator *interpolator,
        /* [annotation][retval][out] */ 
        IUIAnimationTransition **transition) = 0;
        
    };
typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0012_0001
    {	UI_ANIMATION_IDLE_BEHAVIOR_CONTINUE	= 0,
        UI_ANIMATION_IDLE_BEHAVIOR_DISABLE	= 1
    } 	UI_ANIMATION_IDLE_BEHAVIOR;
    	
    struct     IUIAnimationTimer : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  SetTimerUpdateHandler( 
        /* [annotation][unique][in] */ 
        IUIAnimationTimerUpdateHandler *updateHandler,
        /* [annotation][in] */ 
        UI_ANIMATION_IDLE_BEHAVIOR idleBehavior) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetTimerEventHandler( 
        /* [annotation][unique][in] */ 
        IUIAnimationTimerEventHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Enable( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  Disable( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  IsEnabled( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  GetTime( 
        /* [annotation][out] */ 
        UI_ANIMATION_SECONDS *seconds) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetFrameRateThreshold( 
        /* [annotation][in] */ 
        UINT32 framesPerSecond) = 0;
        
    };
    
   
    struct     IUIAnimationTimerUpdateHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnUpdate( 
        /* [annotation][in] */ 
        UI_ANIMATION_SECONDS timeNow,
        /* [annotation][retval][out] */ 
        UI_ANIMATION_UPDATE_RESULT *result) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  SetTimerClientEventHandler( 
        /* [annotation][in] */ 
        IUIAnimationTimerClientEventHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  ClearTimerClientEventHandler( void) = 0;
        
    };
typedef /* [public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0014_0001
    {	UI_ANIMATION_TIMER_CLIENT_IDLE	= 0,
        UI_ANIMATION_TIMER_CLIENT_BUSY	= 1
    } 	UI_ANIMATION_TIMER_CLIENT_STATUS;
    struct     IUIAnimationTimerClientEventHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnTimerClientStatusChanged( 
        /* [annotation][in] */ 
        UI_ANIMATION_TIMER_CLIENT_STATUS newStatus,
        /* [annotation][in] */ 
        UI_ANIMATION_TIMER_CLIENT_STATUS previousStatus) = 0;
        
    };
    struct     IUIAnimationTimerEventHandler : public IObjRef
    {
        virtual /* [annotation] */ 
        HRESULT  OnPreUpdate( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  OnPostUpdate( void) = 0;
        
        virtual /* [annotation] */ 
        HRESULT  OnRenderingTooSlow( 
        /* [annotation][in] */ 
        UINT32 framesPerSecond) = 0;
        
    };
}//end of namespace SOUI

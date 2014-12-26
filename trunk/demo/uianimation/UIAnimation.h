

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* Compiler settings for UIAnimation.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __UIAnimation_h__
#define __UIAnimation_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IUIAnimationManager_FWD_DEFINED__
#define __IUIAnimationManager_FWD_DEFINED__
typedef interface IUIAnimationManager IUIAnimationManager;
#endif 	/* __IUIAnimationManager_FWD_DEFINED__ */


#ifndef __IUIAnimationVariable_FWD_DEFINED__
#define __IUIAnimationVariable_FWD_DEFINED__
typedef interface IUIAnimationVariable IUIAnimationVariable;
#endif 	/* __IUIAnimationVariable_FWD_DEFINED__ */


#ifndef __IUIAnimationStoryboard_FWD_DEFINED__
#define __IUIAnimationStoryboard_FWD_DEFINED__
typedef interface IUIAnimationStoryboard IUIAnimationStoryboard;
#endif 	/* __IUIAnimationStoryboard_FWD_DEFINED__ */


#ifndef __IUIAnimationTransition_FWD_DEFINED__
#define __IUIAnimationTransition_FWD_DEFINED__
typedef interface IUIAnimationTransition IUIAnimationTransition;
#endif 	/* __IUIAnimationTransition_FWD_DEFINED__ */


#ifndef __IUIAnimationManagerEventHandler_FWD_DEFINED__
#define __IUIAnimationManagerEventHandler_FWD_DEFINED__
typedef interface IUIAnimationManagerEventHandler IUIAnimationManagerEventHandler;
#endif 	/* __IUIAnimationManagerEventHandler_FWD_DEFINED__ */


#ifndef __IUIAnimationVariableChangeHandler_FWD_DEFINED__
#define __IUIAnimationVariableChangeHandler_FWD_DEFINED__
typedef interface IUIAnimationVariableChangeHandler IUIAnimationVariableChangeHandler;
#endif 	/* __IUIAnimationVariableChangeHandler_FWD_DEFINED__ */


#ifndef __IUIAnimationVariableIntegerChangeHandler_FWD_DEFINED__
#define __IUIAnimationVariableIntegerChangeHandler_FWD_DEFINED__
typedef interface IUIAnimationVariableIntegerChangeHandler IUIAnimationVariableIntegerChangeHandler;
#endif 	/* __IUIAnimationVariableIntegerChangeHandler_FWD_DEFINED__ */


#ifndef __IUIAnimationStoryboardEventHandler_FWD_DEFINED__
#define __IUIAnimationStoryboardEventHandler_FWD_DEFINED__
typedef interface IUIAnimationStoryboardEventHandler IUIAnimationStoryboardEventHandler;
#endif 	/* __IUIAnimationStoryboardEventHandler_FWD_DEFINED__ */


#ifndef __IUIAnimationPriorityComparison_FWD_DEFINED__
#define __IUIAnimationPriorityComparison_FWD_DEFINED__
typedef interface IUIAnimationPriorityComparison IUIAnimationPriorityComparison;
#endif 	/* __IUIAnimationPriorityComparison_FWD_DEFINED__ */


#ifndef __IUIAnimationTransitionLibrary_FWD_DEFINED__
#define __IUIAnimationTransitionLibrary_FWD_DEFINED__
typedef interface IUIAnimationTransitionLibrary IUIAnimationTransitionLibrary;
#endif 	/* __IUIAnimationTransitionLibrary_FWD_DEFINED__ */


#ifndef __IUIAnimationInterpolator_FWD_DEFINED__
#define __IUIAnimationInterpolator_FWD_DEFINED__
typedef interface IUIAnimationInterpolator IUIAnimationInterpolator;
#endif 	/* __IUIAnimationInterpolator_FWD_DEFINED__ */


#ifndef __IUIAnimationTransitionFactory_FWD_DEFINED__
#define __IUIAnimationTransitionFactory_FWD_DEFINED__
typedef interface IUIAnimationTransitionFactory IUIAnimationTransitionFactory;
#endif 	/* __IUIAnimationTransitionFactory_FWD_DEFINED__ */


#ifndef __IUIAnimationTimer_FWD_DEFINED__
#define __IUIAnimationTimer_FWD_DEFINED__
typedef interface IUIAnimationTimer IUIAnimationTimer;
#endif 	/* __IUIAnimationTimer_FWD_DEFINED__ */


#ifndef __IUIAnimationTimerUpdateHandler_FWD_DEFINED__
#define __IUIAnimationTimerUpdateHandler_FWD_DEFINED__
typedef interface IUIAnimationTimerUpdateHandler IUIAnimationTimerUpdateHandler;
#endif 	/* __IUIAnimationTimerUpdateHandler_FWD_DEFINED__ */


#ifndef __IUIAnimationTimerClientEventHandler_FWD_DEFINED__
#define __IUIAnimationTimerClientEventHandler_FWD_DEFINED__
typedef interface IUIAnimationTimerClientEventHandler IUIAnimationTimerClientEventHandler;
#endif 	/* __IUIAnimationTimerClientEventHandler_FWD_DEFINED__ */


#ifndef __IUIAnimationTimerEventHandler_FWD_DEFINED__
#define __IUIAnimationTimerEventHandler_FWD_DEFINED__
typedef interface IUIAnimationTimerEventHandler IUIAnimationTimerEventHandler;
#endif 	/* __IUIAnimationTimerEventHandler_FWD_DEFINED__ */


#ifndef __UIAnimationManager_FWD_DEFINED__
#define __UIAnimationManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class UIAnimationManager UIAnimationManager;
#else
typedef struct UIAnimationManager UIAnimationManager;
#endif /* __cplusplus */

#endif 	/* __UIAnimationManager_FWD_DEFINED__ */


#ifndef __UIAnimationTransitionLibrary_FWD_DEFINED__
#define __UIAnimationTransitionLibrary_FWD_DEFINED__

#ifdef __cplusplus
typedef class UIAnimationTransitionLibrary UIAnimationTransitionLibrary;
#else
typedef struct UIAnimationTransitionLibrary UIAnimationTransitionLibrary;
#endif /* __cplusplus */

#endif 	/* __UIAnimationTransitionLibrary_FWD_DEFINED__ */


#ifndef __UIAnimationTransitionFactory_FWD_DEFINED__
#define __UIAnimationTransitionFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class UIAnimationTransitionFactory UIAnimationTransitionFactory;
#else
typedef struct UIAnimationTransitionFactory UIAnimationTransitionFactory;
#endif /* __cplusplus */

#endif 	/* __UIAnimationTransitionFactory_FWD_DEFINED__ */


#ifndef __UIAnimationTimer_FWD_DEFINED__
#define __UIAnimationTimer_FWD_DEFINED__

#ifdef __cplusplus
typedef class UIAnimationTimer UIAnimationTimer;
#else
typedef struct UIAnimationTimer UIAnimationTimer;
#endif /* __cplusplus */

#endif 	/* __UIAnimationTimer_FWD_DEFINED__ */


/* header files for imported files */
#include "wtypes.h"
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_UIAnimation_0000_0000 */
/* [local] */ 

//--------------------------------------------------------------------------
//
//  UIAnimation.h
//
//  Windows Animation interface definitions and related types and enums
//  (Generated from UIAnimation.idl)
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable:4001) 
#pragma once
#pragma warning(pop)
















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



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0000_v0_0_s_ifspec;

#ifndef __IUIAnimationManager_INTERFACE_DEFINED__
#define __IUIAnimationManager_INTERFACE_DEFINED__

/* interface IUIAnimationManager */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9169896C-AC8D-4e7d-94E5-67FA4DC2F2E8")
    IUIAnimationManager : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateAnimationVariable( 
            /* [annotation][in] */ 
            __in  DOUBLE initialValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationVariable **variable) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE ScheduleTransition( 
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateStoryboard( 
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationStoryboard **storyboard) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE FinishAllStoryboards( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS completionDeadline) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE AbandonAllStoryboards( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Update( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow,
            /* [annotation][defaultvalue][out] */ 
            __out_opt  UI_ANIMATION_UPDATE_RESULT *updateResult = 0) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetVariableFromTag( 
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationVariable **variable) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetStoryboardFromTag( 
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationStoryboard **storyboard) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetStatus( 
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_MANAGER_STATUS *status) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetAnimationMode( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_MODE mode) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetManagerEventHandler( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationManagerEventHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetCancelPriorityComparison( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetTrimPriorityComparison( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetCompressPriorityComparison( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetConcludePriorityComparison( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetDefaultLongestAcceptableDelay( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS delay) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationManager * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationManager * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateAnimationVariable )( 
            IUIAnimationManager * This,
            /* [annotation][in] */ 
            __in  DOUBLE initialValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationVariable **variable);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *ScheduleTransition )( 
            IUIAnimationManager * This,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateStoryboard )( 
            IUIAnimationManager * This,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationStoryboard **storyboard);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *FinishAllStoryboards )( 
            IUIAnimationManager * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS completionDeadline);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *AbandonAllStoryboards )( 
            IUIAnimationManager * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Update )( 
            IUIAnimationManager * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow,
            /* [annotation][defaultvalue][out] */ 
            __out_opt  UI_ANIMATION_UPDATE_RESULT *updateResult);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetVariableFromTag )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationVariable **variable);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetStoryboardFromTag )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationStoryboard **storyboard);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IUIAnimationManager * This,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_MANAGER_STATUS *status);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetAnimationMode )( 
            IUIAnimationManager * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_MODE mode);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IUIAnimationManager * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IUIAnimationManager * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetManagerEventHandler )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationManagerEventHandler *handler);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetCancelPriorityComparison )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetTrimPriorityComparison )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetCompressPriorityComparison )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetConcludePriorityComparison )( 
            IUIAnimationManager * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationPriorityComparison *comparison);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetDefaultLongestAcceptableDelay )( 
            IUIAnimationManager * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS delay);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            IUIAnimationManager * This);
        
        END_INTERFACE
    } IUIAnimationManagerVtbl;

    interface IUIAnimationManager
    {
        CONST_VTBL struct IUIAnimationManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationManager_CreateAnimationVariable(This,initialValue,variable)	\
    ( (This)->lpVtbl -> CreateAnimationVariable(This,initialValue,variable) ) 

#define IUIAnimationManager_ScheduleTransition(This,variable,transition,timeNow)	\
    ( (This)->lpVtbl -> ScheduleTransition(This,variable,transition,timeNow) ) 

#define IUIAnimationManager_CreateStoryboard(This,storyboard)	\
    ( (This)->lpVtbl -> CreateStoryboard(This,storyboard) ) 

#define IUIAnimationManager_FinishAllStoryboards(This,completionDeadline)	\
    ( (This)->lpVtbl -> FinishAllStoryboards(This,completionDeadline) ) 

#define IUIAnimationManager_AbandonAllStoryboards(This)	\
    ( (This)->lpVtbl -> AbandonAllStoryboards(This) ) 

#define IUIAnimationManager_Update(This,timeNow,updateResult)	\
    ( (This)->lpVtbl -> Update(This,timeNow,updateResult) ) 

#define IUIAnimationManager_GetVariableFromTag(This,object,id,variable)	\
    ( (This)->lpVtbl -> GetVariableFromTag(This,object,id,variable) ) 

#define IUIAnimationManager_GetStoryboardFromTag(This,object,id,storyboard)	\
    ( (This)->lpVtbl -> GetStoryboardFromTag(This,object,id,storyboard) ) 

#define IUIAnimationManager_GetStatus(This,status)	\
    ( (This)->lpVtbl -> GetStatus(This,status) ) 

#define IUIAnimationManager_SetAnimationMode(This,mode)	\
    ( (This)->lpVtbl -> SetAnimationMode(This,mode) ) 

#define IUIAnimationManager_Pause(This)	\
    ( (This)->lpVtbl -> Pause(This) ) 

#define IUIAnimationManager_Resume(This)	\
    ( (This)->lpVtbl -> Resume(This) ) 

#define IUIAnimationManager_SetManagerEventHandler(This,handler)	\
    ( (This)->lpVtbl -> SetManagerEventHandler(This,handler) ) 

#define IUIAnimationManager_SetCancelPriorityComparison(This,comparison)	\
    ( (This)->lpVtbl -> SetCancelPriorityComparison(This,comparison) ) 

#define IUIAnimationManager_SetTrimPriorityComparison(This,comparison)	\
    ( (This)->lpVtbl -> SetTrimPriorityComparison(This,comparison) ) 

#define IUIAnimationManager_SetCompressPriorityComparison(This,comparison)	\
    ( (This)->lpVtbl -> SetCompressPriorityComparison(This,comparison) ) 

#define IUIAnimationManager_SetConcludePriorityComparison(This,comparison)	\
    ( (This)->lpVtbl -> SetConcludePriorityComparison(This,comparison) ) 

#define IUIAnimationManager_SetDefaultLongestAcceptableDelay(This,delay)	\
    ( (This)->lpVtbl -> SetDefaultLongestAcceptableDelay(This,delay) ) 

#define IUIAnimationManager_Shutdown(This)	\
    ( (This)->lpVtbl -> Shutdown(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationManager_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_UIAnimation_0000_0001 */
/* [local] */ 

typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0001_0001
    {	UI_ANIMATION_ROUNDING_NEAREST	= 0,
	UI_ANIMATION_ROUNDING_FLOOR	= 1,
	UI_ANIMATION_ROUNDING_CEILING	= 2
    } 	UI_ANIMATION_ROUNDING_MODE;



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0001_v0_0_s_ifspec;

#ifndef __IUIAnimationVariable_INTERFACE_DEFINED__
#define __IUIAnimationVariable_INTERFACE_DEFINED__

/* interface IUIAnimationVariable */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationVariable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8CEEB155-2849-4ce5-9448-91FF70E1E4D9")
    IUIAnimationVariable : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetValue( 
            /* [annotation][retval][out] */ 
            __out  DOUBLE *value) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetFinalValue( 
            /* [annotation][retval][out] */ 
            __out  DOUBLE *finalValue) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetPreviousValue( 
            /* [annotation][retval][out] */ 
            __out  DOUBLE *previousValue) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetIntegerValue( 
            /* [annotation][retval][out] */ 
            __out  INT32 *value) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetFinalIntegerValue( 
            /* [annotation][retval][out] */ 
            __out  INT32 *finalValue) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetPreviousIntegerValue( 
            /* [annotation][retval][out] */ 
            __out  INT32 *previousValue) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetCurrentStoryboard( 
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationStoryboard **storyboard) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetLowerBound( 
            /* [annotation][in] */ 
            __in  DOUBLE bound) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetUpperBound( 
            /* [annotation][in] */ 
            __in  DOUBLE bound) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetRoundingMode( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_ROUNDING_MODE mode) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetTag( 
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetTag( 
            /* [annotation][out] */ 
            __deref_opt_out  IUnknown **object,
            /* [annotation][out] */ 
            __out_opt  UINT32 *id) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetVariableChangeHandler( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationVariableChangeHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetVariableIntegerChangeHandler( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationVariableIntegerChangeHandler *handler) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationVariableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationVariable * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationVariable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationVariable * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *value);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetFinalValue )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *finalValue);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetPreviousValue )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *previousValue);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetIntegerValue )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __out  INT32 *value);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetFinalIntegerValue )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __out  INT32 *finalValue);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetPreviousIntegerValue )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __out  INT32 *previousValue);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetCurrentStoryboard )( 
            IUIAnimationVariable * This,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationStoryboard **storyboard);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetLowerBound )( 
            IUIAnimationVariable * This,
            /* [annotation][in] */ 
            __in  DOUBLE bound);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetUpperBound )( 
            IUIAnimationVariable * This,
            /* [annotation][in] */ 
            __in  DOUBLE bound);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetRoundingMode )( 
            IUIAnimationVariable * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_ROUNDING_MODE mode);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetTag )( 
            IUIAnimationVariable * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetTag )( 
            IUIAnimationVariable * This,
            /* [annotation][out] */ 
            __deref_opt_out  IUnknown **object,
            /* [annotation][out] */ 
            __out_opt  UINT32 *id);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetVariableChangeHandler )( 
            IUIAnimationVariable * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationVariableChangeHandler *handler);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetVariableIntegerChangeHandler )( 
            IUIAnimationVariable * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationVariableIntegerChangeHandler *handler);
        
        END_INTERFACE
    } IUIAnimationVariableVtbl;

    interface IUIAnimationVariable
    {
        CONST_VTBL struct IUIAnimationVariableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationVariable_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationVariable_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationVariable_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationVariable_GetValue(This,value)	\
    ( (This)->lpVtbl -> GetValue(This,value) ) 

#define IUIAnimationVariable_GetFinalValue(This,finalValue)	\
    ( (This)->lpVtbl -> GetFinalValue(This,finalValue) ) 

#define IUIAnimationVariable_GetPreviousValue(This,previousValue)	\
    ( (This)->lpVtbl -> GetPreviousValue(This,previousValue) ) 

#define IUIAnimationVariable_GetIntegerValue(This,value)	\
    ( (This)->lpVtbl -> GetIntegerValue(This,value) ) 

#define IUIAnimationVariable_GetFinalIntegerValue(This,finalValue)	\
    ( (This)->lpVtbl -> GetFinalIntegerValue(This,finalValue) ) 

#define IUIAnimationVariable_GetPreviousIntegerValue(This,previousValue)	\
    ( (This)->lpVtbl -> GetPreviousIntegerValue(This,previousValue) ) 

#define IUIAnimationVariable_GetCurrentStoryboard(This,storyboard)	\
    ( (This)->lpVtbl -> GetCurrentStoryboard(This,storyboard) ) 

#define IUIAnimationVariable_SetLowerBound(This,bound)	\
    ( (This)->lpVtbl -> SetLowerBound(This,bound) ) 

#define IUIAnimationVariable_SetUpperBound(This,bound)	\
    ( (This)->lpVtbl -> SetUpperBound(This,bound) ) 

#define IUIAnimationVariable_SetRoundingMode(This,mode)	\
    ( (This)->lpVtbl -> SetRoundingMode(This,mode) ) 

#define IUIAnimationVariable_SetTag(This,object,id)	\
    ( (This)->lpVtbl -> SetTag(This,object,id) ) 

#define IUIAnimationVariable_GetTag(This,object,id)	\
    ( (This)->lpVtbl -> GetTag(This,object,id) ) 

#define IUIAnimationVariable_SetVariableChangeHandler(This,handler)	\
    ( (This)->lpVtbl -> SetVariableChangeHandler(This,handler) ) 

#define IUIAnimationVariable_SetVariableIntegerChangeHandler(This,handler)	\
    ( (This)->lpVtbl -> SetVariableIntegerChangeHandler(This,handler) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationVariable_INTERFACE_DEFINED__ */


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



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0002_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0002_v0_0_s_ifspec;

#ifndef __IUIAnimationStoryboard_INTERFACE_DEFINED__
#define __IUIAnimationStoryboard_INTERFACE_DEFINED__

/* interface IUIAnimationStoryboard */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationStoryboard;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A8FF128F-9BF9-4af1-9E67-E5E410DEFB84")
    IUIAnimationStoryboard : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE AddTransition( 
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE AddKeyframeAtOffset( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME existingKeyframe,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_KEYFRAME *keyframe) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE AddKeyframeAfterTransition( 
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_KEYFRAME *keyframe) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE AddTransitionAtKeyframe( 
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME startKeyframe) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE AddTransitionBetweenKeyframes( 
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME startKeyframe,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME endKeyframe) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE RepeatBetweenKeyframes( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME startKeyframe,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME endKeyframe,
            /* [annotation][in] */ 
            __in  INT32 repetitionCount) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE HoldVariable( 
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetLongestAcceptableDelay( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS delay) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Schedule( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow,
            /* [annotation][defaultvalue][out] */ 
            __out_opt  UI_ANIMATION_SCHEDULING_RESULT *schedulingResult = 0) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Conclude( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Finish( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS completionDeadline) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Abandon( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetTag( 
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetTag( 
            /* [annotation][out] */ 
            __deref_opt_out  IUnknown **object,
            /* [annotation][out] */ 
            __out_opt  UINT32 *id) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetStatus( 
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_STORYBOARD_STATUS *status) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetElapsedTime( 
            /* [annotation][out] */ 
            __out  UI_ANIMATION_SECONDS *elapsedTime) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetStoryboardEventHandler( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationStoryboardEventHandler *handler) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationStoryboardVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationStoryboard * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationStoryboard * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationStoryboard * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *AddTransition )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *AddKeyframeAtOffset )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME existingKeyframe,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_KEYFRAME *keyframe);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *AddKeyframeAfterTransition )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_KEYFRAME *keyframe);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *AddTransitionAtKeyframe )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME startKeyframe);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *AddTransitionBetweenKeyframes )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  IUIAnimationTransition *transition,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME startKeyframe,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME endKeyframe);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *RepeatBetweenKeyframes )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME startKeyframe,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_KEYFRAME endKeyframe,
            /* [annotation][in] */ 
            __in  INT32 repetitionCount);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *HoldVariable )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetLongestAcceptableDelay )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS delay);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Schedule )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow,
            /* [annotation][defaultvalue][out] */ 
            __out_opt  UI_ANIMATION_SCHEDULING_RESULT *schedulingResult);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Conclude )( 
            IUIAnimationStoryboard * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Finish )( 
            IUIAnimationStoryboard * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS completionDeadline);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Abandon )( 
            IUIAnimationStoryboard * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetTag )( 
            IUIAnimationStoryboard * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUnknown *object,
            /* [annotation][in] */ 
            __in  UINT32 id);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetTag )( 
            IUIAnimationStoryboard * This,
            /* [annotation][out] */ 
            __deref_opt_out  IUnknown **object,
            /* [annotation][out] */ 
            __out_opt  UINT32 *id);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetStatus )( 
            IUIAnimationStoryboard * This,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_STORYBOARD_STATUS *status);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetElapsedTime )( 
            IUIAnimationStoryboard * This,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_SECONDS *elapsedTime);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetStoryboardEventHandler )( 
            IUIAnimationStoryboard * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationStoryboardEventHandler *handler);
        
        END_INTERFACE
    } IUIAnimationStoryboardVtbl;

    interface IUIAnimationStoryboard
    {
        CONST_VTBL struct IUIAnimationStoryboardVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationStoryboard_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationStoryboard_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationStoryboard_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationStoryboard_AddTransition(This,variable,transition)	\
    ( (This)->lpVtbl -> AddTransition(This,variable,transition) ) 

#define IUIAnimationStoryboard_AddKeyframeAtOffset(This,existingKeyframe,offset,keyframe)	\
    ( (This)->lpVtbl -> AddKeyframeAtOffset(This,existingKeyframe,offset,keyframe) ) 

#define IUIAnimationStoryboard_AddKeyframeAfterTransition(This,transition,keyframe)	\
    ( (This)->lpVtbl -> AddKeyframeAfterTransition(This,transition,keyframe) ) 

#define IUIAnimationStoryboard_AddTransitionAtKeyframe(This,variable,transition,startKeyframe)	\
    ( (This)->lpVtbl -> AddTransitionAtKeyframe(This,variable,transition,startKeyframe) ) 

#define IUIAnimationStoryboard_AddTransitionBetweenKeyframes(This,variable,transition,startKeyframe,endKeyframe)	\
    ( (This)->lpVtbl -> AddTransitionBetweenKeyframes(This,variable,transition,startKeyframe,endKeyframe) ) 

#define IUIAnimationStoryboard_RepeatBetweenKeyframes(This,startKeyframe,endKeyframe,repetitionCount)	\
    ( (This)->lpVtbl -> RepeatBetweenKeyframes(This,startKeyframe,endKeyframe,repetitionCount) ) 

#define IUIAnimationStoryboard_HoldVariable(This,variable)	\
    ( (This)->lpVtbl -> HoldVariable(This,variable) ) 

#define IUIAnimationStoryboard_SetLongestAcceptableDelay(This,delay)	\
    ( (This)->lpVtbl -> SetLongestAcceptableDelay(This,delay) ) 

#define IUIAnimationStoryboard_Schedule(This,timeNow,schedulingResult)	\
    ( (This)->lpVtbl -> Schedule(This,timeNow,schedulingResult) ) 

#define IUIAnimationStoryboard_Conclude(This)	\
    ( (This)->lpVtbl -> Conclude(This) ) 

#define IUIAnimationStoryboard_Finish(This,completionDeadline)	\
    ( (This)->lpVtbl -> Finish(This,completionDeadline) ) 

#define IUIAnimationStoryboard_Abandon(This)	\
    ( (This)->lpVtbl -> Abandon(This) ) 

#define IUIAnimationStoryboard_SetTag(This,object,id)	\
    ( (This)->lpVtbl -> SetTag(This,object,id) ) 

#define IUIAnimationStoryboard_GetTag(This,object,id)	\
    ( (This)->lpVtbl -> GetTag(This,object,id) ) 

#define IUIAnimationStoryboard_GetStatus(This,status)	\
    ( (This)->lpVtbl -> GetStatus(This,status) ) 

#define IUIAnimationStoryboard_GetElapsedTime(This,elapsedTime)	\
    ( (This)->lpVtbl -> GetElapsedTime(This,elapsedTime) ) 

#define IUIAnimationStoryboard_SetStoryboardEventHandler(This,handler)	\
    ( (This)->lpVtbl -> SetStoryboardEventHandler(This,handler) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationStoryboard_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationTransition_INTERFACE_DEFINED__
#define __IUIAnimationTransition_INTERFACE_DEFINED__

/* interface IUIAnimationTransition */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTransition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DC6CE252-F731-41cf-B610-614B6CA049AD")
    IUIAnimationTransition : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetInitialValue( 
            /* [annotation][in] */ 
            __in  DOUBLE value) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetInitialVelocity( 
            /* [annotation][in] */ 
            __in  DOUBLE velocity) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE IsDurationKnown( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetDuration( 
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_SECONDS *duration) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTransitionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTransition * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTransition * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTransition * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetInitialValue )( 
            IUIAnimationTransition * This,
            /* [annotation][in] */ 
            __in  DOUBLE value);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetInitialVelocity )( 
            IUIAnimationTransition * This,
            /* [annotation][in] */ 
            __in  DOUBLE velocity);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *IsDurationKnown )( 
            IUIAnimationTransition * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetDuration )( 
            IUIAnimationTransition * This,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_SECONDS *duration);
        
        END_INTERFACE
    } IUIAnimationTransitionVtbl;

    interface IUIAnimationTransition
    {
        CONST_VTBL struct IUIAnimationTransitionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTransition_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTransition_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTransition_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTransition_SetInitialValue(This,value)	\
    ( (This)->lpVtbl -> SetInitialValue(This,value) ) 

#define IUIAnimationTransition_SetInitialVelocity(This,velocity)	\
    ( (This)->lpVtbl -> SetInitialVelocity(This,velocity) ) 

#define IUIAnimationTransition_IsDurationKnown(This)	\
    ( (This)->lpVtbl -> IsDurationKnown(This) ) 

#define IUIAnimationTransition_GetDuration(This,duration)	\
    ( (This)->lpVtbl -> GetDuration(This,duration) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTransition_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationManagerEventHandler_INTERFACE_DEFINED__
#define __IUIAnimationManagerEventHandler_INTERFACE_DEFINED__

/* interface IUIAnimationManagerEventHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationManagerEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("783321ED-78A3-4366-B574-6AF607A64788")
    IUIAnimationManagerEventHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnManagerStatusChanged( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_MANAGER_STATUS newStatus,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_MANAGER_STATUS previousStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationManagerEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationManagerEventHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationManagerEventHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationManagerEventHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnManagerStatusChanged )( 
            IUIAnimationManagerEventHandler * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_MANAGER_STATUS newStatus,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_MANAGER_STATUS previousStatus);
        
        END_INTERFACE
    } IUIAnimationManagerEventHandlerVtbl;

    interface IUIAnimationManagerEventHandler
    {
        CONST_VTBL struct IUIAnimationManagerEventHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationManagerEventHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationManagerEventHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationManagerEventHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationManagerEventHandler_OnManagerStatusChanged(This,newStatus,previousStatus)	\
    ( (This)->lpVtbl -> OnManagerStatusChanged(This,newStatus,previousStatus) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationManagerEventHandler_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationVariableChangeHandler_INTERFACE_DEFINED__
#define __IUIAnimationVariableChangeHandler_INTERFACE_DEFINED__

/* interface IUIAnimationVariableChangeHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationVariableChangeHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6358B7BA-87D2-42d5-BF71-82E919DD5862")
    IUIAnimationVariableChangeHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnValueChanged( 
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  DOUBLE newValue,
            /* [annotation][in] */ 
            __in  DOUBLE previousValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationVariableChangeHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationVariableChangeHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationVariableChangeHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationVariableChangeHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnValueChanged )( 
            IUIAnimationVariableChangeHandler * This,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  DOUBLE newValue,
            /* [annotation][in] */ 
            __in  DOUBLE previousValue);
        
        END_INTERFACE
    } IUIAnimationVariableChangeHandlerVtbl;

    interface IUIAnimationVariableChangeHandler
    {
        CONST_VTBL struct IUIAnimationVariableChangeHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationVariableChangeHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationVariableChangeHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationVariableChangeHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationVariableChangeHandler_OnValueChanged(This,storyboard,variable,newValue,previousValue)	\
    ( (This)->lpVtbl -> OnValueChanged(This,storyboard,variable,newValue,previousValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationVariableChangeHandler_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationVariableIntegerChangeHandler_INTERFACE_DEFINED__
#define __IUIAnimationVariableIntegerChangeHandler_INTERFACE_DEFINED__

/* interface IUIAnimationVariableIntegerChangeHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationVariableIntegerChangeHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BB3E1550-356E-44b0-99DA-85AC6017865E")
    IUIAnimationVariableIntegerChangeHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnIntegerValueChanged( 
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  INT32 newValue,
            /* [annotation][in] */ 
            __in  INT32 previousValue) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationVariableIntegerChangeHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationVariableIntegerChangeHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationVariableIntegerChangeHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationVariableIntegerChangeHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnIntegerValueChanged )( 
            IUIAnimationVariableIntegerChangeHandler * This,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard,
            /* [annotation][in] */ 
            __in  IUIAnimationVariable *variable,
            /* [annotation][in] */ 
            __in  INT32 newValue,
            /* [annotation][in] */ 
            __in  INT32 previousValue);
        
        END_INTERFACE
    } IUIAnimationVariableIntegerChangeHandlerVtbl;

    interface IUIAnimationVariableIntegerChangeHandler
    {
        CONST_VTBL struct IUIAnimationVariableIntegerChangeHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationVariableIntegerChangeHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationVariableIntegerChangeHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationVariableIntegerChangeHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationVariableIntegerChangeHandler_OnIntegerValueChanged(This,storyboard,variable,newValue,previousValue)	\
    ( (This)->lpVtbl -> OnIntegerValueChanged(This,storyboard,variable,newValue,previousValue) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationVariableIntegerChangeHandler_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationStoryboardEventHandler_INTERFACE_DEFINED__
#define __IUIAnimationStoryboardEventHandler_INTERFACE_DEFINED__

/* interface IUIAnimationStoryboardEventHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationStoryboardEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3D5C9008-EC7C-4364-9F8A-9AF3C58CBAE6")
    IUIAnimationStoryboardEventHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnStoryboardStatusChanged( 
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_STORYBOARD_STATUS newStatus,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_STORYBOARD_STATUS previousStatus) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnStoryboardUpdated( 
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationStoryboardEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationStoryboardEventHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationStoryboardEventHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationStoryboardEventHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnStoryboardStatusChanged )( 
            IUIAnimationStoryboardEventHandler * This,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_STORYBOARD_STATUS newStatus,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_STORYBOARD_STATUS previousStatus);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnStoryboardUpdated )( 
            IUIAnimationStoryboardEventHandler * This,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *storyboard);
        
        END_INTERFACE
    } IUIAnimationStoryboardEventHandlerVtbl;

    interface IUIAnimationStoryboardEventHandler
    {
        CONST_VTBL struct IUIAnimationStoryboardEventHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationStoryboardEventHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationStoryboardEventHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationStoryboardEventHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationStoryboardEventHandler_OnStoryboardStatusChanged(This,storyboard,newStatus,previousStatus)	\
    ( (This)->lpVtbl -> OnStoryboardStatusChanged(This,storyboard,newStatus,previousStatus) ) 

#define IUIAnimationStoryboardEventHandler_OnStoryboardUpdated(This,storyboard)	\
    ( (This)->lpVtbl -> OnStoryboardUpdated(This,storyboard) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationStoryboardEventHandler_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_UIAnimation_0000_0008 */
/* [local] */ 

typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0008_0001
    {	UI_ANIMATION_PRIORITY_EFFECT_FAILURE	= 0,
	UI_ANIMATION_PRIORITY_EFFECT_DELAY	= 1
    } 	UI_ANIMATION_PRIORITY_EFFECT;



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0008_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0008_v0_0_s_ifspec;

#ifndef __IUIAnimationPriorityComparison_INTERFACE_DEFINED__
#define __IUIAnimationPriorityComparison_INTERFACE_DEFINED__

/* interface IUIAnimationPriorityComparison */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationPriorityComparison;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("83FA9B74-5F86-4618-BC6A-A2FAC19B3F44")
    IUIAnimationPriorityComparison : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE HasPriority( 
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *scheduledStoryboard,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *newStoryboard,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_PRIORITY_EFFECT priorityEffect) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationPriorityComparisonVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationPriorityComparison * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationPriorityComparison * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationPriorityComparison * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *HasPriority )( 
            IUIAnimationPriorityComparison * This,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *scheduledStoryboard,
            /* [annotation][in] */ 
            __in  IUIAnimationStoryboard *newStoryboard,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_PRIORITY_EFFECT priorityEffect);
        
        END_INTERFACE
    } IUIAnimationPriorityComparisonVtbl;

    interface IUIAnimationPriorityComparison
    {
        CONST_VTBL struct IUIAnimationPriorityComparisonVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationPriorityComparison_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationPriorityComparison_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationPriorityComparison_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationPriorityComparison_HasPriority(This,scheduledStoryboard,newStoryboard,priorityEffect)	\
    ( (This)->lpVtbl -> HasPriority(This,scheduledStoryboard,newStoryboard,priorityEffect) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationPriorityComparison_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_UIAnimation_0000_0009 */
/* [local] */ 

typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0009_0001
    {	UI_ANIMATION_SLOPE_INCREASING	= 0,
	UI_ANIMATION_SLOPE_DECREASING	= 1
    } 	UI_ANIMATION_SLOPE;



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0009_v0_0_s_ifspec;

#ifndef __IUIAnimationTransitionLibrary_INTERFACE_DEFINED__
#define __IUIAnimationTransitionLibrary_INTERFACE_DEFINED__

/* interface IUIAnimationTransitionLibrary */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTransitionLibrary;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CA5A14B1-D24F-48b8-8FE4-C78169BA954E")
    IUIAnimationTransitionLibrary : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateInstantaneousTransition( 
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateConstantTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateDiscreteTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS delay,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS hold,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateLinearTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateLinearTransitionFromSpeed( 
            /* [annotation][in] */ 
            __in  DOUBLE speed,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateSinusoidalTransitionFromVelocity( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS period,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateSinusoidalTransitionFromRange( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE minimumValue,
            /* [annotation][in] */ 
            __in  DOUBLE maximumValue,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS period,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SLOPE slope,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateAccelerateDecelerateTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  DOUBLE accelerationRatio,
            /* [annotation][in] */ 
            __in  DOUBLE decelerationRatio,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateReversalTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateCubicTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  DOUBLE finalVelocity,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateSmoothStopTransition( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS maximumDuration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateParabolicTransitionFromAcceleration( 
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  DOUBLE finalVelocity,
            /* [annotation][in] */ 
            __in  DOUBLE acceleration,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTransitionLibraryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTransitionLibrary * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTransitionLibrary * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTransitionLibrary * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateInstantaneousTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateConstantTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateDiscreteTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS delay,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS hold,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateLinearTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateLinearTransitionFromSpeed )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  DOUBLE speed,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateSinusoidalTransitionFromVelocity )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS period,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateSinusoidalTransitionFromRange )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE minimumValue,
            /* [annotation][in] */ 
            __in  DOUBLE maximumValue,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS period,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SLOPE slope,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateAccelerateDecelerateTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  DOUBLE accelerationRatio,
            /* [annotation][in] */ 
            __in  DOUBLE decelerationRatio,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateReversalTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateCubicTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  DOUBLE finalVelocity,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateSmoothStopTransition )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS maximumDuration,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateParabolicTransitionFromAcceleration )( 
            IUIAnimationTransitionLibrary * This,
            /* [annotation][in] */ 
            __in  DOUBLE finalValue,
            /* [annotation][in] */ 
            __in  DOUBLE finalVelocity,
            /* [annotation][in] */ 
            __in  DOUBLE acceleration,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        END_INTERFACE
    } IUIAnimationTransitionLibraryVtbl;

    interface IUIAnimationTransitionLibrary
    {
        CONST_VTBL struct IUIAnimationTransitionLibraryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTransitionLibrary_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTransitionLibrary_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTransitionLibrary_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTransitionLibrary_CreateInstantaneousTransition(This,finalValue,transition)	\
    ( (This)->lpVtbl -> CreateInstantaneousTransition(This,finalValue,transition) ) 

#define IUIAnimationTransitionLibrary_CreateConstantTransition(This,duration,transition)	\
    ( (This)->lpVtbl -> CreateConstantTransition(This,duration,transition) ) 

#define IUIAnimationTransitionLibrary_CreateDiscreteTransition(This,delay,finalValue,hold,transition)	\
    ( (This)->lpVtbl -> CreateDiscreteTransition(This,delay,finalValue,hold,transition) ) 

#define IUIAnimationTransitionLibrary_CreateLinearTransition(This,duration,finalValue,transition)	\
    ( (This)->lpVtbl -> CreateLinearTransition(This,duration,finalValue,transition) ) 

#define IUIAnimationTransitionLibrary_CreateLinearTransitionFromSpeed(This,speed,finalValue,transition)	\
    ( (This)->lpVtbl -> CreateLinearTransitionFromSpeed(This,speed,finalValue,transition) ) 

#define IUIAnimationTransitionLibrary_CreateSinusoidalTransitionFromVelocity(This,duration,period,transition)	\
    ( (This)->lpVtbl -> CreateSinusoidalTransitionFromVelocity(This,duration,period,transition) ) 

#define IUIAnimationTransitionLibrary_CreateSinusoidalTransitionFromRange(This,duration,minimumValue,maximumValue,period,slope,transition)	\
    ( (This)->lpVtbl -> CreateSinusoidalTransitionFromRange(This,duration,minimumValue,maximumValue,period,slope,transition) ) 

#define IUIAnimationTransitionLibrary_CreateAccelerateDecelerateTransition(This,duration,finalValue,accelerationRatio,decelerationRatio,transition)	\
    ( (This)->lpVtbl -> CreateAccelerateDecelerateTransition(This,duration,finalValue,accelerationRatio,decelerationRatio,transition) ) 

#define IUIAnimationTransitionLibrary_CreateReversalTransition(This,duration,transition)	\
    ( (This)->lpVtbl -> CreateReversalTransition(This,duration,transition) ) 

#define IUIAnimationTransitionLibrary_CreateCubicTransition(This,duration,finalValue,finalVelocity,transition)	\
    ( (This)->lpVtbl -> CreateCubicTransition(This,duration,finalValue,finalVelocity,transition) ) 

#define IUIAnimationTransitionLibrary_CreateSmoothStopTransition(This,maximumDuration,finalValue,transition)	\
    ( (This)->lpVtbl -> CreateSmoothStopTransition(This,maximumDuration,finalValue,transition) ) 

#define IUIAnimationTransitionLibrary_CreateParabolicTransitionFromAcceleration(This,finalValue,finalVelocity,acceleration,transition)	\
    ( (This)->lpVtbl -> CreateParabolicTransitionFromAcceleration(This,finalValue,finalVelocity,acceleration,transition) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTransitionLibrary_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_UIAnimation_0000_0010 */
/* [local] */ 

typedef /* [public][public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0010_0001
    {	UI_ANIMATION_DEPENDENCY_NONE	= 0,
	UI_ANIMATION_DEPENDENCY_INTERMEDIATE_VALUES	= 0x1,
	UI_ANIMATION_DEPENDENCY_FINAL_VALUE	= 0x2,
	UI_ANIMATION_DEPENDENCY_FINAL_VELOCITY	= 0x4,
	UI_ANIMATION_DEPENDENCY_DURATION	= 0x8
    } 	UI_ANIMATION_DEPENDENCIES;

// DEFINE_ENUM_FLAG_OPERATORS(UI_ANIMATION_DEPENDENCIES);


extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0010_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0010_v0_0_s_ifspec;

#ifndef __IUIAnimationInterpolator_INTERFACE_DEFINED__
#define __IUIAnimationInterpolator_INTERFACE_DEFINED__

/* interface IUIAnimationInterpolator */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationInterpolator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7815CBBA-DDF7-478c-A46C-7B6C738B7978")
    IUIAnimationInterpolator : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetInitialValueAndVelocity( 
            /* [annotation][in] */ 
            __in  DOUBLE initialValue,
            /* [annotation][in] */ 
            __in  DOUBLE initialVelocity) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetDuration( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetDuration( 
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_SECONDS *duration) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetFinalValue( 
            /* [annotation][retval][out] */ 
            __out  DOUBLE *value) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE InterpolateValue( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *value) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE InterpolateVelocity( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *velocity) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetDependencies( 
            /* [annotation][out] */ 
            __out  UI_ANIMATION_DEPENDENCIES *initialValueDependencies,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_DEPENDENCIES *initialVelocityDependencies,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_DEPENDENCIES *durationDependencies) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationInterpolatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationInterpolator * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationInterpolator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationInterpolator * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetInitialValueAndVelocity )( 
            IUIAnimationInterpolator * This,
            /* [annotation][in] */ 
            __in  DOUBLE initialValue,
            /* [annotation][in] */ 
            __in  DOUBLE initialVelocity);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetDuration )( 
            IUIAnimationInterpolator * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS duration);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetDuration )( 
            IUIAnimationInterpolator * This,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_SECONDS *duration);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetFinalValue )( 
            IUIAnimationInterpolator * This,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *value);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *InterpolateValue )( 
            IUIAnimationInterpolator * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *value);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *InterpolateVelocity )( 
            IUIAnimationInterpolator * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS offset,
            /* [annotation][retval][out] */ 
            __out  DOUBLE *velocity);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetDependencies )( 
            IUIAnimationInterpolator * This,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_DEPENDENCIES *initialValueDependencies,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_DEPENDENCIES *initialVelocityDependencies,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_DEPENDENCIES *durationDependencies);
        
        END_INTERFACE
    } IUIAnimationInterpolatorVtbl;

    interface IUIAnimationInterpolator
    {
        CONST_VTBL struct IUIAnimationInterpolatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationInterpolator_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationInterpolator_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationInterpolator_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationInterpolator_SetInitialValueAndVelocity(This,initialValue,initialVelocity)	\
    ( (This)->lpVtbl -> SetInitialValueAndVelocity(This,initialValue,initialVelocity) ) 

#define IUIAnimationInterpolator_SetDuration(This,duration)	\
    ( (This)->lpVtbl -> SetDuration(This,duration) ) 

#define IUIAnimationInterpolator_GetDuration(This,duration)	\
    ( (This)->lpVtbl -> GetDuration(This,duration) ) 

#define IUIAnimationInterpolator_GetFinalValue(This,value)	\
    ( (This)->lpVtbl -> GetFinalValue(This,value) ) 

#define IUIAnimationInterpolator_InterpolateValue(This,offset,value)	\
    ( (This)->lpVtbl -> InterpolateValue(This,offset,value) ) 

#define IUIAnimationInterpolator_InterpolateVelocity(This,offset,velocity)	\
    ( (This)->lpVtbl -> InterpolateVelocity(This,offset,velocity) ) 

#define IUIAnimationInterpolator_GetDependencies(This,initialValueDependencies,initialVelocityDependencies,durationDependencies)	\
    ( (This)->lpVtbl -> GetDependencies(This,initialValueDependencies,initialVelocityDependencies,durationDependencies) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationInterpolator_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationTransitionFactory_INTERFACE_DEFINED__
#define __IUIAnimationTransitionFactory_INTERFACE_DEFINED__

/* interface IUIAnimationTransitionFactory */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTransitionFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FCD91E03-3E3B-45ad-BBB1-6DFC8153743D")
    IUIAnimationTransitionFactory : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE CreateTransition( 
            /* [annotation][in] */ 
            __in  IUIAnimationInterpolator *interpolator,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTransitionFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTransitionFactory * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTransitionFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTransitionFactory * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *CreateTransition )( 
            IUIAnimationTransitionFactory * This,
            /* [annotation][in] */ 
            __in  IUIAnimationInterpolator *interpolator,
            /* [annotation][retval][out] */ 
            __deref_out  IUIAnimationTransition **transition);
        
        END_INTERFACE
    } IUIAnimationTransitionFactoryVtbl;

    interface IUIAnimationTransitionFactory
    {
        CONST_VTBL struct IUIAnimationTransitionFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTransitionFactory_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTransitionFactory_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTransitionFactory_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTransitionFactory_CreateTransition(This,interpolator,transition)	\
    ( (This)->lpVtbl -> CreateTransition(This,interpolator,transition) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTransitionFactory_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_UIAnimation_0000_0012 */
/* [local] */ 

typedef /* [public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0012_0001
    {	UI_ANIMATION_IDLE_BEHAVIOR_CONTINUE	= 0,
	UI_ANIMATION_IDLE_BEHAVIOR_DISABLE	= 1
    } 	UI_ANIMATION_IDLE_BEHAVIOR;



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0012_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0012_v0_0_s_ifspec;

#ifndef __IUIAnimationTimer_INTERFACE_DEFINED__
#define __IUIAnimationTimer_INTERFACE_DEFINED__

/* interface IUIAnimationTimer */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTimer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6B0EFAD1-A053-41d6-9085-33A689144665")
    IUIAnimationTimer : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetTimerUpdateHandler( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationTimerUpdateHandler *updateHandler,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_IDLE_BEHAVIOR idleBehavior) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetTimerEventHandler( 
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationTimerEventHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Enable( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE Disable( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE IsEnabled( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE GetTime( 
            /* [annotation][out] */ 
            __out  UI_ANIMATION_SECONDS *seconds) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetFrameRateThreshold( 
            /* [annotation][in] */ 
            __in  UINT32 framesPerSecond) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTimerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTimer * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTimer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTimer * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetTimerUpdateHandler )( 
            IUIAnimationTimer * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationTimerUpdateHandler *updateHandler,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_IDLE_BEHAVIOR idleBehavior);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetTimerEventHandler )( 
            IUIAnimationTimer * This,
            /* [annotation][unique][in] */ 
            __in_opt  IUIAnimationTimerEventHandler *handler);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Enable )( 
            IUIAnimationTimer * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *Disable )( 
            IUIAnimationTimer * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *IsEnabled )( 
            IUIAnimationTimer * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *GetTime )( 
            IUIAnimationTimer * This,
            /* [annotation][out] */ 
            __out  UI_ANIMATION_SECONDS *seconds);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetFrameRateThreshold )( 
            IUIAnimationTimer * This,
            /* [annotation][in] */ 
            __in  UINT32 framesPerSecond);
        
        END_INTERFACE
    } IUIAnimationTimerVtbl;

    interface IUIAnimationTimer
    {
        CONST_VTBL struct IUIAnimationTimerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTimer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTimer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTimer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTimer_SetTimerUpdateHandler(This,updateHandler,idleBehavior)	\
    ( (This)->lpVtbl -> SetTimerUpdateHandler(This,updateHandler,idleBehavior) ) 

#define IUIAnimationTimer_SetTimerEventHandler(This,handler)	\
    ( (This)->lpVtbl -> SetTimerEventHandler(This,handler) ) 

#define IUIAnimationTimer_Enable(This)	\
    ( (This)->lpVtbl -> Enable(This) ) 

#define IUIAnimationTimer_Disable(This)	\
    ( (This)->lpVtbl -> Disable(This) ) 

#define IUIAnimationTimer_IsEnabled(This)	\
    ( (This)->lpVtbl -> IsEnabled(This) ) 

#define IUIAnimationTimer_GetTime(This,seconds)	\
    ( (This)->lpVtbl -> GetTime(This,seconds) ) 

#define IUIAnimationTimer_SetFrameRateThreshold(This,framesPerSecond)	\
    ( (This)->lpVtbl -> SetFrameRateThreshold(This,framesPerSecond) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTimer_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationTimerUpdateHandler_INTERFACE_DEFINED__
#define __IUIAnimationTimerUpdateHandler_INTERFACE_DEFINED__

/* interface IUIAnimationTimerUpdateHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTimerUpdateHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("195509B7-5D5E-4e3e-B278-EE3759B367AD")
    IUIAnimationTimerUpdateHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnUpdate( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_UPDATE_RESULT *result) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE SetTimerClientEventHandler( 
            /* [annotation][in] */ 
            __in  IUIAnimationTimerClientEventHandler *handler) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE ClearTimerClientEventHandler( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTimerUpdateHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTimerUpdateHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTimerUpdateHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTimerUpdateHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnUpdate )( 
            IUIAnimationTimerUpdateHandler * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_SECONDS timeNow,
            /* [annotation][retval][out] */ 
            __out  UI_ANIMATION_UPDATE_RESULT *result);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *SetTimerClientEventHandler )( 
            IUIAnimationTimerUpdateHandler * This,
            /* [annotation][in] */ 
            __in  IUIAnimationTimerClientEventHandler *handler);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *ClearTimerClientEventHandler )( 
            IUIAnimationTimerUpdateHandler * This);
        
        END_INTERFACE
    } IUIAnimationTimerUpdateHandlerVtbl;

    interface IUIAnimationTimerUpdateHandler
    {
        CONST_VTBL struct IUIAnimationTimerUpdateHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTimerUpdateHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTimerUpdateHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTimerUpdateHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTimerUpdateHandler_OnUpdate(This,timeNow,result)	\
    ( (This)->lpVtbl -> OnUpdate(This,timeNow,result) ) 

#define IUIAnimationTimerUpdateHandler_SetTimerClientEventHandler(This,handler)	\
    ( (This)->lpVtbl -> SetTimerClientEventHandler(This,handler) ) 

#define IUIAnimationTimerUpdateHandler_ClearTimerClientEventHandler(This)	\
    ( (This)->lpVtbl -> ClearTimerClientEventHandler(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTimerUpdateHandler_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_UIAnimation_0000_0014 */
/* [local] */ 

typedef /* [public][public][public][v1_enum] */ 
enum __MIDL___MIDL_itf_UIAnimation_0000_0014_0001
    {	UI_ANIMATION_TIMER_CLIENT_IDLE	= 0,
	UI_ANIMATION_TIMER_CLIENT_BUSY	= 1
    } 	UI_ANIMATION_TIMER_CLIENT_STATUS;



extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0014_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_UIAnimation_0000_0014_v0_0_s_ifspec;

#ifndef __IUIAnimationTimerClientEventHandler_INTERFACE_DEFINED__
#define __IUIAnimationTimerClientEventHandler_INTERFACE_DEFINED__

/* interface IUIAnimationTimerClientEventHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTimerClientEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BEDB4DB6-94FA-4bfb-A47F-EF2D9E408C25")
    IUIAnimationTimerClientEventHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnTimerClientStatusChanged( 
            /* [annotation][in] */ 
            __in  UI_ANIMATION_TIMER_CLIENT_STATUS newStatus,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_TIMER_CLIENT_STATUS previousStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTimerClientEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTimerClientEventHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTimerClientEventHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTimerClientEventHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnTimerClientStatusChanged )( 
            IUIAnimationTimerClientEventHandler * This,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_TIMER_CLIENT_STATUS newStatus,
            /* [annotation][in] */ 
            __in  UI_ANIMATION_TIMER_CLIENT_STATUS previousStatus);
        
        END_INTERFACE
    } IUIAnimationTimerClientEventHandlerVtbl;

    interface IUIAnimationTimerClientEventHandler
    {
        CONST_VTBL struct IUIAnimationTimerClientEventHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTimerClientEventHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTimerClientEventHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTimerClientEventHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTimerClientEventHandler_OnTimerClientStatusChanged(This,newStatus,previousStatus)	\
    ( (This)->lpVtbl -> OnTimerClientStatusChanged(This,newStatus,previousStatus) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTimerClientEventHandler_INTERFACE_DEFINED__ */


#ifndef __IUIAnimationTimerEventHandler_INTERFACE_DEFINED__
#define __IUIAnimationTimerEventHandler_INTERFACE_DEFINED__

/* interface IUIAnimationTimerEventHandler */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IUIAnimationTimerEventHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("274A7DEA-D771-4095-ABBD-8DF7ABD23CE3")
    IUIAnimationTimerEventHandler : public IUnknown
    {
    public:
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnPreUpdate( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnPostUpdate( void) = 0;
        
        virtual /* [annotation] */ 
        __checkReturn
        HRESULT STDMETHODCALLTYPE OnRenderingTooSlow( 
            /* [annotation][in] */ 
            __in  UINT32 framesPerSecond) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IUIAnimationTimerEventHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUIAnimationTimerEventHandler * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUIAnimationTimerEventHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUIAnimationTimerEventHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnPreUpdate )( 
            IUIAnimationTimerEventHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnPostUpdate )( 
            IUIAnimationTimerEventHandler * This);
        
        /* [annotation] */ 
        __checkReturn
        HRESULT ( STDMETHODCALLTYPE *OnRenderingTooSlow )( 
            IUIAnimationTimerEventHandler * This,
            /* [annotation][in] */ 
            __in  UINT32 framesPerSecond);
        
        END_INTERFACE
    } IUIAnimationTimerEventHandlerVtbl;

    interface IUIAnimationTimerEventHandler
    {
        CONST_VTBL struct IUIAnimationTimerEventHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUIAnimationTimerEventHandler_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IUIAnimationTimerEventHandler_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IUIAnimationTimerEventHandler_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IUIAnimationTimerEventHandler_OnPreUpdate(This)	\
    ( (This)->lpVtbl -> OnPreUpdate(This) ) 

#define IUIAnimationTimerEventHandler_OnPostUpdate(This)	\
    ( (This)->lpVtbl -> OnPostUpdate(This) ) 

#define IUIAnimationTimerEventHandler_OnRenderingTooSlow(This,framesPerSecond)	\
    ( (This)->lpVtbl -> OnRenderingTooSlow(This,framesPerSecond) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IUIAnimationTimerEventHandler_INTERFACE_DEFINED__ */



#ifndef __UIAnimation_LIBRARY_DEFINED__
#define __UIAnimation_LIBRARY_DEFINED__

/* library UIAnimation */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_UIAnimation;

EXTERN_C const CLSID CLSID_UIAnimationManager;

#ifdef __cplusplus

class DECLSPEC_UUID("4C1FC63A-695C-47E8-A339-1A194BE3D0B8")
UIAnimationManager;
#endif

EXTERN_C const CLSID CLSID_UIAnimationTransitionLibrary;

#ifdef __cplusplus

class DECLSPEC_UUID("1D6322AD-AA85-4EF5-A828-86D71067D145")
UIAnimationTransitionLibrary;
#endif

EXTERN_C const CLSID CLSID_UIAnimationTransitionFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("8A9B1CDD-FCD7-419c-8B44-42FD17DB1887")
UIAnimationTransitionFactory;
#endif

EXTERN_C const CLSID CLSID_UIAnimationTimer;

#ifdef __cplusplus

class DECLSPEC_UUID("BFCD4A0C-06B6-4384-B768-0DAA792C380E")
UIAnimationTimer;
#endif
#endif /* __UIAnimation_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif




/**
* Copyright (C) 2014-2050 SOUI group
* All rights reserved.
* 
* @file       imgdecoder-i.h
* @brief      
* @version    v1.0      
* @author     soui group    
* @date       2014-05-28
* 
* Describe    
*/

#pragma once

#include "obj-ref-i.h"

namespace SOUI
{
    /**
    * @struct     IImgFrame
    * @brief      a image frame
    * 
    * Describe    
    */
    struct IImgFrame
    {
        /**
         * GetSize
         * @brief    get image size in pixel
         * @param [out]   UINT * pWid --  image width
         * @param [out]   UINT * pHei --  image height
         * @return   BOOL -- TRUE: succeed; FALSE: failed
         * Describe  
         */    
        virtual BOOL GetSize(UINT *pWid,UINT *pHei)=0;

        /**
         * CopyPixels
         * @brief    copy pixel data to the output buffer
         * @param [in] const RECT * prc --  The rectangle to copy. A NULL value specifies the entire bitmap.
         * @param [in] UINT cbStride --  The stride of the bitmap
         * @param [in] UINT cbBufferSize --  The size of the buffer.
         * @param [out] BYTE * pbBuffer --  A pointer to the buffer
         * @return   BOOL  TRUE if successful, or FALSE otherwise. 
         * Describe  
         */    
        virtual BOOL CopyPixels( 
            /* [unique][in] */ const RECT *prc,
            /* [in] */ UINT cbStride,
            /* [in] */ UINT cbBufferSize,
            /* [size_is][out] */ BYTE *pbBuffer) = 0;

        /**
         * GetDelay
         * @brief    get delay for a frame of a gif image
         * @return   int time in MS*10
         * Describe  
         */    
        virtual int GetDelay() = 0;
    };

    /**
    * @struct     IImgX
    * @brief      image data
    * 
    * Describe    
    */
    struct IImgX : public IObjRef
    {
        /**
         * LoadFromMemory
         * @brief    load image data from a mememory buffer
         * @param    void * pBuf --  point of buffer
         * @param    size_t bufLen --  size of buffer
         * @return   int 
         * Describe  
         */    
        virtual int LoadFromMemory(void *pBuf,size_t bufLen)=0;
        /**
         * LoadFromFile
         * @brief    load image from file
         * @param    LPCWSTR pszFileName --  file name in unicode
         * @return   int
         * Describe  
         */    
        virtual int LoadFromFile(LPCWSTR pszFileName)=0;

        /**
         * LoadFromFile
         * @brief    load image from file
         * @param    LPCSTR pszFileName --  file name in char
         * @return   int
         * Describe  
         */    
        virtual int LoadFromFile(LPCSTR pszFileName)=0;
        
        /**
         * GetFrameCount
         * @brief    get frame count of the image data
         * @return   UINT -- image frame count
         * Describe  
         */    
        virtual UINT GetFrameCount()=0;

        /**
         * GetFrame
         * @brief    get frame data
         * @param    UINT iFrame -- the target frame index 
         * @return   IImgFrame * -- the associated image frame with the input frame index
         * Describe  
         */    
        virtual IImgFrame * GetFrame(UINT iFrame)=0;
    };

    /**
    * @struct     IImgDecoderFactory
    * @brief      image decoder factory
    * 
    * Describe    
    */
    struct IImgDecoderFactory : public IObjRef
    {
        /**
         * IsAlphaPremultiple
         * @brief    get whether the output image buffer should be alpha premultipled.
         * @return   BOOL -- TRUE: the output image will be alpha premultipled.
         * Describe  
         */    
        virtual BOOL IsAlphaPremultiple()=0;

        /**
         * SetAlphaPremultiple
         * @brief    set alpha premultiple flag
         * @param    BOOL bPreMultiple --  alpha premultiple flag
         * @return   void
         * Describe  
         */    
        virtual void SetAlphaPremultiple(BOOL bPreMultiple)=0;

        /**
         * CreateImgX
         * @brief    create a IImgX object
         * @param [out] IImgX * * ppImgDecoder --  the created IImgX
         * @return   BOOL 
         * Describe  
         */    
        virtual BOOL CreateImgX(IImgX **ppImgDecoder)=0;
    };
}
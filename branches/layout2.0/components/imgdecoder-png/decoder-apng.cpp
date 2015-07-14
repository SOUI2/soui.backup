#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <png.h>
#include <pngstruct.h>
#include <pnginfo.h>

#include "decoder-apng.h"

struct IPngReader
{
    virtual png_size_t read(png_bytep data, png_size_t length) = 0;
};

struct IPngReader_Mem : public IPngReader
{
    const char *pbuf;
    png_size_t   nLen;
    
    IPngReader_Mem(const char *_pbuf,png_size_t _nLen):pbuf(_pbuf),nLen(_nLen){}
    png_size_t read(png_bytep data, png_size_t length)
    {
        if(nLen < length) length = nLen;
        memcpy(data,pbuf,length);
        pbuf += length;
        nLen -= length;
        return length;
    }
};

struct IPngReader_File: public IPngReader
{
    FILE *f;
    IPngReader_File(FILE *_f):f(_f){}
    
    png_size_t read(png_bytep data, png_size_t length)
    {
        return fread(data,1,length,f);
    }
};

void 
mypng_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   if (png_ptr == NULL)
      return;
   IPngReader * pMem = (IPngReader*)png_ptr->io_ptr;
   png_size_t rc = pMem->read(data,length);
   if(rc < length)
   {
       png_error(png_ptr,"read error");
   }
}

APNGDATA * loadPng(IPngReader *pSrc)
{
	png_bytep  data;
	png_uint_32 bytesPerRow;
	png_uint_32 bytesPerFrame;
    png_bytepp rowPointers;
	png_byte   sig[8];
	
	png_structp png_ptr_read;
	png_infop info_ptr_read;
	
    pSrc->read(sig,8);
    if(!png_check_sig(sig,8))
    {
        return NULL;
    }
    
    png_ptr_read = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    info_ptr_read = png_create_info_struct(png_ptr_read);
    
	if (setjmp(png_jmpbuf(png_ptr_read)))
    {
        png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
        return NULL;
    }
        
    png_set_read_fn(png_ptr_read,pSrc,mypng_read_data);
    png_set_sig_bytes(png_ptr_read, 8);
 
    if ((png_ptr_read->bit_depth < 8) ||
        (png_ptr_read->color_type == PNG_COLOR_TYPE_PALETTE) ||
        (info_ptr_read->valid & PNG_INFO_tRNS))
        png_set_expand(png_ptr_read);

    png_set_add_alpha(png_ptr_read, 0xff, PNG_FILLER_AFTER);
    png_set_interlace_handling(png_ptr_read);
    png_set_gray_to_rgb(png_ptr_read);
    png_set_strip_16(png_ptr_read);
   
	png_read_info(png_ptr_read, info_ptr_read);
    png_read_update_info(png_ptr_read, info_ptr_read);
    
    bytesPerRow = png_ptr_read->width * 4;
    bytesPerFrame = bytesPerRow * png_ptr_read->height;
    
    APNGDATA * apng = (APNGDATA*) malloc(sizeof(APNGDATA));
    memset(apng,0,sizeof(APNGDATA));
    apng->nWid  = png_ptr_read->width;
    apng->nHei = png_ptr_read->height;
    

	if (!png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_acTL))
	{//load png doesn't has this trunk.
	    data = (png_bytep)malloc(bytesPerRow * apng->nHei);
	    memset(data,0,bytesPerFrame);
        rowPointers = (png_bytepp)malloc(sizeof(png_bytep)* apng->nHei);
        for(int i=0;i<apng->nHei;i++)
            rowPointers[i] = data + bytesPerRow * i;
        
        png_read_image(png_ptr_read,rowPointers);
                
        apng->pdata =data;
        apng->nFrames =1;
	}else
	{//load apng
        apng->nFrames  = png_get_num_frames(png_ptr_read, info_ptr_read);

        data = (png_bytep)malloc( bytesPerFrame * apng->nFrames);
        memset(data,0,bytesPerFrame * apng->nFrames);
        
        rowPointers = (png_bytepp)malloc(sizeof(png_bytep)* apng->nHei*apng->nFrames);
        for(int i=0;i<info_ptr_read->height*apng->nFrames;i++)
            rowPointers[i] = data + bytesPerRow * i;
        
        apng->pdata =data;
        apng->nLoops = png_get_num_plays(png_ptr_read, info_ptr_read);
        apng->pDelay = (unsigned short*)malloc(sizeof(unsigned short)*apng->nFrames);
        
        for(int iFrame = 0;iFrame<apng->nFrames;iFrame++)
        {
            png_read_frame_head(png_ptr_read, info_ptr_read);
            
            if (png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_fcTL))
            {
                png_uint_16 delay_num = info_ptr_read->next_frame_delay_num,
                            delay_den = info_ptr_read->next_frame_delay_den;
            
                if (delay_den==0 || delay_den==100)
                    apng->pDelay[iFrame] = delay_num;
                else
                    if (delay_den==10)
                        apng->pDelay[iFrame] = delay_num*10;
                    else
                        if (delay_den==1000)
                            apng->pDelay[iFrame] = delay_num/10;
                        else
                            apng->pDelay[iFrame] = delay_num*100/delay_den;
            }else
            {
                apng->pDelay[iFrame] = 0;
            }
            png_read_image(png_ptr_read, rowPointers + iFrame * apng->nHei);
        }
	}
	png_read_end(png_ptr_read,info_ptr_read);
	
    free(rowPointers);
    png_destroy_read_struct(&png_ptr_read, &info_ptr_read, NULL);
    return apng;    
}

APNGDATA * LoadAPNG_from_file( const wchar_t * pszFileName )
{
    FILE *f = _wfopen(pszFileName,L"rb");
    if(!f) return NULL;
    IPngReader_File file(f);
    APNGDATA *pRet = loadPng(&file);
    fclose(f);
    return pRet;
}

APNGDATA * LoadAPNG_from_memory(const char * pBuf, size_t nLen )
{
    IPngReader_Mem mem(pBuf,nLen);
    return loadPng(&mem);
}

void APNG_Destroy( APNGDATA *apng )
{
    if(apng)
    {
        if(apng->pdata) free(apng->pdata);
        if(apng->pDelay) free(apng->pDelay);
        free(apng);
    }
}

bool SavePng(const unsigned char *pData, int nWid,int nHei,int nStride,const wchar_t * pszFileName)
{
    
    return false;
}


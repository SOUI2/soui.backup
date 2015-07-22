#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <png.h>
#include <pngstruct.h>
#include <pnginfo.h>

#include "decoder-apng.h"
#include <assert.h>

#define SASSERT(x) assert(x)

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
	png_bytep  dataFrame;
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
    
    dataFrame = (png_bytep)malloc(bytesPerRow * apng->nHei);
    memset(dataFrame,0,bytesPerFrame);
    
    rowPointers = (png_bytepp)malloc(sizeof(png_bytep)* apng->nHei);
    for(int i=0;i<apng->nHei;i++)
        rowPointers[i] = dataFrame + bytesPerRow * i;

	if (!png_get_valid(png_ptr_read, info_ptr_read, PNG_INFO_acTL))
	{//load png doesn't has this trunk.
        
        png_read_image(png_ptr_read,rowPointers);
                
        apng->pdata =dataFrame;
        apng->nFrames =1;
	}else
	{//load apng
        apng->nFrames  = png_get_num_frames(png_ptr_read, info_ptr_read);

        png_bytep data = (png_bytep)malloc( bytesPerFrame * apng->nFrames);
        png_bytep dataCur = (png_bytep)malloc(bytesPerFrame);
        
        memset(dataCur,0,bytesPerFrame);
               
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
            png_read_image(png_ptr_read, rowPointers);
            
            png_bytep lineDst=dataCur+info_ptr_read->next_frame_y_offset*bytesPerRow;
            //准备好背景
            switch(info_ptr_read->next_frame_dispose_op)
            {
            case PNG_DISPOSE_OP_BACKGROUND://clear background
                for(int y=0;y<info_ptr_read->next_frame_height;y++)
                {
                    memset(lineDst+y*bytesPerRow+info_ptr_read->next_frame_x_offset*4,0,info_ptr_read->next_frame_width*4);
                }
                break;
            case PNG_DISPOSE_OP_PREVIOUS://copy previous frame
                {
                    SASSERT(iFrame>=2);
                    png_bytep lineSour = data + (iFrame-1)*bytesPerFrame + info_ptr_read->next_frame_y_offset*bytesPerRow;
                    for(int y=0;y<info_ptr_read->next_frame_height;y++)
                    {
                        memcpy(lineDst+y*bytesPerRow+info_ptr_read->next_frame_x_offset*4,
                            lineSour+y*bytesPerRow+info_ptr_read->next_frame_x_offset*4,
                            info_ptr_read->next_frame_width*4
                            );
                    }
                }
                break;
            case PNG_DISPOSE_OP_NONE://using current frame, doing nothing
                break;
            default:
                SASSERT(0);
                break;
            }

            png_bytep lineSour=dataFrame;

            //根据指定的混合方式，和背景和混合
            switch(info_ptr_read->next_frame_blend_op)
            {
            case PNG_BLEND_OP_OVER:
                {
                    for(int y=0;y<info_ptr_read->next_frame_height;y++)
                    {
                        png_bytep lineDst1=lineDst + info_ptr_read->next_frame_x_offset*4;
                        png_bytep lineSour1=lineSour;
                        for(int x=0;x<info_ptr_read->next_frame_width;x++)
                        {
                            png_byte alpha = lineSour1[3];
                            lineDst1[0] = (lineDst1[0]*(255-alpha) +lineSour1[0]*alpha)/255;
                            lineDst1[1] = (lineDst1[1]*(255-alpha) +lineSour1[1]*alpha)/255;
                            lineDst1[2] = (lineDst1[2]*(255-alpha) +lineSour1[2]*alpha)/255;
                            lineDst1[3] = alpha;
                            
                            lineDst1 += 4;
                            lineSour1 += 4;
//                             *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
//                             *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
//                             *lineDst1++ = ((*lineDst1)*(255-alpha)+(*lineSour1++)*alpha)>>8;
//                             *lineDst1++ = *lineSour1++;
                        }
                        lineDst += bytesPerRow;
                        lineSour+= bytesPerRow;
                    }
                }
                break;
            case PNG_BLEND_OP_SOURCE:
                {
                    for(int y=0;y<info_ptr_read->next_frame_height;y++)
                    {
                        png_bytep lineDst1=lineDst + info_ptr_read->next_frame_x_offset*4;
                        png_bytep lineSour1=lineSour;
                        memcpy(lineDst1,lineSour1,info_ptr_read->next_frame_width*4);
                        lineDst += bytesPerRow;
                        lineSour+= bytesPerRow;
                    }
                }
                break;
            default:
                SASSERT(FALSE);
                break;
            }
            memcpy(data + iFrame*bytesPerFrame, dataCur, bytesPerFrame);       
            //*/     
        }
        free(dataFrame);
        free(dataCur);
        apng->pdata =data;
	}
    free(rowPointers);

	png_read_end(png_ptr_read,info_ptr_read);
	
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


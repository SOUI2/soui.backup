/* apng2gif GUI version 1.6
 *
 * This program converts APNG animations into animated GIF format.
 * Wu64 quantizer is used for true-color files.
 *
 * http://apng2gif.sourceforge.net/
 *
 * Copyright (c) 2010-2013 Max Stepin
 * maxst at users.sourceforge.net
 *
 * zlib license
 * ------------
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
using namespace std;
#include "png.h"     /* original (unpatched) libpng is ok */
#include "zlib.h"
#include <malloc.h>

#include "decoder-apng.h"

#define swap16(data) _byteswap_ushort(data)
#define swap32(data) _byteswap_ulong(data)

#define notabc(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

#define id_IHDR 0x52444849
#define id_acTL 0x4C546361
#define id_fcTL 0x4C546366
#define id_IDAT 0x54414449
#define id_fdAT 0x54416466
#define id_IEND 0x444E4549


struct CHUNK { unsigned char * p; unsigned int size; };

void Frame_Create(FRAME *frm,int nWid,int nHei)
{
    int rowSize = nWid * 4;
    frm->p = (BYTE*)malloc(rowSize*nHei);
    frm->rows = (LPBYTE *)malloc(nHei*sizeof(LPBYTE));
    LPBYTE  t=frm->p;
    for(int i=0;i<nHei;i++,t+=rowSize)
    {
        frm->rows[i] = t;
    }
}

void Frame_Destroy(FRAME *frm)
{
    if(frm->p) free(frm->p);
    frm->p=NULL;
    if(frm->rows) free(frm->rows);
    frm->rows=NULL;
}


APNGDATA::APNGDATA()
{
    frame.p = NULL;
    frame.rows = NULL;
    pDelay =NULL;
    nWid = nHei =nFrames = nLoops =0;
}

APNGDATA::~APNGDATA()
{
    Frame_Destroy(&frame);
    if(pDelay) delete []pDelay;
}

FRAME frameRaw;

void info_fn(png_structp png_ptr, png_infop info_ptr)
{
  png_set_expand(png_ptr);
  png_set_strip_16(png_ptr);
  png_set_gray_to_rgb(png_ptr);
  png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
  (void)png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);
}

void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
  png_progressive_combine_row(png_ptr, frameRaw.rows[row_num], new_row);
}

void compose_frame(unsigned char ** rows_dst, unsigned char ** rows_src, unsigned char bop, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  unsigned int  i, j;
  int u, v, al;

  for (j=0; j<h; j++)
  {
    unsigned char * sp = rows_src[j];
    unsigned char * dp = rows_dst[j+y] + x*4;

    if (bop == 0)
      memcpy(dp, sp, w*4);
    else
    for (i=0; i<w; i++, sp+=4, dp+=4)
    {
      if (sp[3] == 255)
        memcpy(dp, sp, 4);
      else
      if (sp[3] != 0)
      {
        if (dp[3] != 0)
        {
          u = sp[3]*255;
          v = (255-sp[3])*dp[3];
          al = 255*255-(255-sp[3])*(255-dp[3]);
          dp[0] = (sp[0]*u + dp[0]*v)/al;
          dp[1] = (sp[1]*u + dp[1]*v)/al;
          dp[2] = (sp[2]*u + dp[2]*v)/al;
          dp[3] = al/255;
        }
        else
          memcpy(dp, sp, 4);
      }
    }
  }
}


void recalc_crc(unsigned char * p, unsigned int size)
{
  unsigned int crc = crc32(0, Z_NULL, 0);
  crc = crc32(crc, p + 4, size - 8);
  crc = swap32(crc);
  memcpy(p + size - 4, &crc, 4);
}

struct IReader
{
	virtual bool end() = 0;
	virtual int read(void *pbuf, int nSize) =0;
	virtual void seek(int nPos, unsigned int mode) = 0;
	virtual int tell() = 0;
};

class CMemReader : public IReader
{
public:
	CMemReader() :m_pBuf(NULL), m_nSize(0), m_nPos(0)
	{}

	void Attach(const char *pBuf, int nSize)
	{
		m_pBuf = pBuf;
		m_nSize = nSize;
		m_nPos = 0;
	}

	const char * Detach()
	{
		const char *pRet = m_pBuf;
		m_pBuf = NULL;
		m_nSize = 0;
		m_nPos = 0;
		return pRet;
	}

	virtual bool end()
	{
		return m_nPos == m_nSize;
	}

	virtual int read(void *pbuf, int nSize)
	{
		if (!m_pBuf) return 0;
		int nRemain = m_nSize - m_nPos;
		int nRet = min(nSize, nRemain);
		memcpy(pbuf, m_pBuf+m_nPos, nRet);
		m_nPos += nRet;
		return nRet;
	}

	virtual void seek(int nPos, unsigned int mode)
	{
		switch (mode)
		{
		case SEEK_SET:
			m_nPos = nPos;
			break;
		case SEEK_END:
			m_nPos = m_nSize;
			break;
		case SEEK_CUR:
			m_nPos += nPos;
			break;
		}
		if (m_nPos < 0) m_nPos = 0;
		if (m_nPos > m_nSize) m_nPos = m_nSize;
	}

	virtual int tell()
	{
		return m_nPos;
	}
protected:
	const char *m_pBuf;
	int   m_nSize;
	int   m_nPos;
};

class CFileReader : public IReader
{
public:
	CFileReader() :_f(NULL){}
	~CFileReader()
	{
		close();
	}

	bool open(LPCWSTR pszFileName)
	{
		if (_f) return false;
		_f = _wfopen(pszFileName, L"rb");
		return !!_f;
	}
	void close()
	{
		if (_f) fclose(_f);
		_f = NULL;
	}

	virtual bool end(){ return feof(_f); }
	virtual int read(void *pbuf, int nSize)
	{
		return fread(pbuf, 1, nSize, _f);
	}
	virtual void seek(int nPos, unsigned int nOrigin)
	{
		::fseek(_f, nPos, nOrigin);
	}
	virtual int tell()
	{
		return ftell(_f);
	}
protected:
	FILE * _f;
};

unsigned int read_chunk(IReader *pReader, CHUNK * pChunk)
{
	unsigned int len;
	if (pReader->read(&len,4) == 4)
	{
		pChunk->size = swap32(len) + 12;
		pChunk->p = new unsigned char[pChunk->size];
		unsigned int * pi = (unsigned int *)pChunk->p;
		pi[0] = len;
		if (pReader->read(pChunk->p + 4, pChunk->size - 4) == pChunk->size - 4)
			return pi[1];
	}
	return 0;
}

APNGDATA * LoadAPNG(IReader *pReader)
{
    FRAME framesAPNG;         // holds all APNG frames
    unsigned short * pDelays;

    unsigned int   id, i, j, w0, h0, x0, y0;
    unsigned int   delay_num, delay_den, dop, bop, rowbytes, imagesize;
    unsigned int * pi;
    CHUNK          chunk_ihdr;
    CHUNK          chunk;
    png_structp    png_ptr;
    png_infop      info_ptr;
    unsigned char  sig[8];
    unsigned char  header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    unsigned char  footer[12] = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};
    unsigned int   w = 0;
    unsigned int   h = 0;
    unsigned int   num_frames = 1;
    unsigned int   num_loops = 0;
    unsigned int   flag_actl = 0;
    unsigned int   fctl_count = 0;
    unsigned int   flag_idat = 0;
    unsigned int   flag_info = 0;
    FRAME          frameCur = {0};
    FRAME          frameNext = {0};
    int            res = 0;
    vector<CHUNK> info_chunks;

    if (pReader->read(sig, 8) == 8 && memcmp(sig, header, 8) == 0)
    {
        id = read_chunk(pReader, &chunk_ihdr);

        if (id == id_IHDR && chunk_ihdr.size == 25)
        {
            pi = (unsigned int *)chunk_ihdr.p;
            w0 = w = swap32(pi[2]);
            h0 = h = swap32(pi[3]);
            x0 = 0;
            y0 = 0;
            dop = 0;
            bop = 0;
            rowbytes = w * 4;
            imagesize = h * rowbytes;

            pDelays = new unsigned short[num_frames];
            pDelays[0] = 10;
            
            Frame_Create(&frameRaw,w,h);
            Frame_Create(&framesAPNG,w,h);
            
            frameCur = framesAPNG;

            png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            info_ptr = png_create_info_struct(png_ptr);
            setjmp(png_jmpbuf(png_ptr));
            png_set_progressive_read_fn(png_ptr, NULL, info_fn, row_fn, NULL);
            png_process_data(png_ptr, info_ptr, &header[0], 8);
            png_process_data(png_ptr, info_ptr, chunk_ihdr.p, chunk_ihdr.size);
            flag_info = 0;

            while ( !pReader->end() )
            {
                id = read_chunk(pReader, &chunk);
                pi = (unsigned int *)chunk.p;

                if (id == id_acTL)
                {
                    flag_actl = 1;
                    num_frames = swap32(pi[2]);
                    num_loops  = swap32(pi[3]);

                    delete[] pDelays;
                    pDelays = new unsigned short[num_frames];

                    Frame_Destroy(&framesAPNG);
                    Frame_Create(&framesAPNG,w,h*num_frames);
                    
                    frameCur = framesAPNG;

                    delete[] chunk.p;
                }
                else
                    if (id == id_fcTL)
                    {
                        if (fctl_count)
                        {
                            png_process_data(png_ptr, info_ptr, &footer[0], 12);
                            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

                            frameNext.p = frameCur.p + imagesize;
                            frameNext.rows = frameCur.rows + h;

                            if (dop == 2)
                                memcpy(frameNext.p, frameCur.p, imagesize);

                            compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);

                            if (dop != 2)
                            {
                                memcpy(frameNext.p, frameCur.p, imagesize);
                                if (dop == 1)
                                    for (j=0; j<h0; j++)
                                        memset(frameNext.rows[y0 + j] + x0*4, 0, w0*4);
                            }
                            frameCur = frameNext;

                            memcpy(chunk_ihdr.p + 8, chunk.p + 12, 8);
                            recalc_crc(chunk_ihdr.p, chunk_ihdr.size);

                            png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                            info_ptr = png_create_info_struct(png_ptr);
                            setjmp(png_jmpbuf(png_ptr));
                            png_set_progressive_read_fn(png_ptr, NULL, info_fn, row_fn, NULL);
                            png_process_data(png_ptr, info_ptr, &header[0], 8);
                            png_process_data(png_ptr, info_ptr, chunk_ihdr.p, chunk_ihdr.size);
                            flag_info = 0;
                        }

                        w0 = swap32(pi[3]);
                        h0 = swap32(pi[4]);
                        x0 = swap32(pi[5]);
                        y0 = swap32(pi[6]);
                        delay_num = chunk.p[28]*256 + chunk.p[29];
                        delay_den = chunk.p[30]*256 + chunk.p[31];

                        if (delay_den==0 || delay_den==100)
                            pDelays[fctl_count] = delay_num;
                        else
                            if (delay_den==10)
                                pDelays[fctl_count] = delay_num*10;
                            else
                                if (delay_den==1000)
                                    pDelays[fctl_count] = delay_num/10;
                                else
                                    pDelays[fctl_count] = delay_num*100/delay_den;

                        dop = chunk.p[32];
                        bop = chunk.p[33];
                        if (!fctl_count)
                        {
                            bop = 0;
                            if (dop == 2)
                                dop = 1;
                        }
                        fctl_count++;
                        delete[] chunk.p;
                    }
                    else
                        if (id == id_IDAT)
                        {
                            flag_idat = 1;
                            if (fctl_count || !flag_actl)
                            {
                                if (!flag_info)
                                {
                                    flag_info = 1;
                                    for (i=0; i<info_chunks.size(); ++i)
                                        png_process_data(png_ptr, info_ptr, info_chunks[i].p, info_chunks[i].size);
                                }
                                png_process_data(png_ptr, info_ptr, chunk.p, chunk.size);
                            }
                            delete[] chunk.p;
                        }
                        else
                            if (id == id_fdAT)
                            {
                                flag_idat = 1;
                                if (!flag_info)
                                {
                                    flag_info = 1;
                                    for (i=0; i<info_chunks.size(); ++i)
                                        png_process_data(png_ptr, info_ptr, info_chunks[i].p, info_chunks[i].size);
                                }
                                pi[1] = swap32(chunk.size - 16);
                                pi[2] = id_IDAT;
                                recalc_crc(chunk.p + 4, chunk.size - 4);
                                png_process_data(png_ptr, info_ptr, chunk.p + 4, chunk.size - 4);
                                delete[] chunk.p;
                            }
                            else
                                if (id == id_IEND)
                                {
                                    png_process_data(png_ptr, info_ptr, &footer[0], 12);

                                    compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
                                    delete[] chunk.p;
                                    break;
                                }
                                else
                                    if (notabc(chunk.p[4]) || notabc(chunk.p[5]) || notabc(chunk.p[6]) || notabc(chunk.p[7]))
                                    {
                                        delete[] chunk.p;
                                        break;
                                    }
                                    else
                                    {
                                        if (!flag_idat)
                                            info_chunks.push_back(chunk);
                                        else
                                            delete[] chunk.p;
                                    }
            }
            Frame_Destroy(&frameRaw);
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        }
        else
            res = 1;
    }
    else
        res = 1;

    for (i=0; i<info_chunks.size(); ++i)
        delete[] info_chunks[i].p;

    info_chunks.clear();
    delete[] chunk_ihdr.p;

    if(res != 0) return NULL;

    APNGDATA *pRet = new APNGDATA;
    pRet->frame = framesAPNG;
    pRet->pDelay = pDelays;
    pRet->nWid = w;
    pRet->nHei = h;
    pRet->nFrames = num_frames;
    pRet->nLoops = num_loops;

    return pRet;
}

APNGDATA * LoadAPNG_from_file( LPCWSTR pszFileName )
{
    CFileReader reader;
    if(!reader.open(pszFileName)) return NULL;
    return LoadAPNG(&reader);
}

APNGDATA * LoadAPNG_from_memory(char * pBuf, int nLen )
{
    CMemReader reader;
    reader.Attach(pBuf,nLen);
    return LoadAPNG(&reader);
}

// 
// 
// DWORD WINAPI ConvertAPNGtoGIF(wchar_t * szIn, wchar_t * szOut, int trans, int blend, int r, int g, int b)
// {
//   unsigned int w, h, num_frames, num_loops;
//   // framesAPNG: a single memory block to keep all APNG frames
// //   if (LoadAPNG(szIn) != 0)
//   {
//     return 1;
//   }
// 
// //   delete[] framesAPNG.rows;
// //   delete[] framesAPNG.p;
// 
// 
//   return 0;
// }

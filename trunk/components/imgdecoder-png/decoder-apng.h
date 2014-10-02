#pragma once

struct FRAME { 
    BYTE * p;       //全部图片数据
    LPBYTE * rows;  //每一行的开始位置，有多帧时按顺序排列行
};

class APNGDATA
{
public:
    APNGDATA();

    ~APNGDATA();

    FRAME frame;
    unsigned short *pDelay;
    int   nWid,nHei;
    int   nFrames;
    int   nLoops;
};

APNGDATA * LoadAPNG_from_file(LPCWSTR pszFileName);

APNGDATA * LoadAPNG_from_memory(char * pBuf, int nLen);

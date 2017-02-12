// residbuilder.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "tinyxml/tinyxml.h"

const wchar_t  RB_HEADER[]=
L"/*<------------------------------------------------------------------------------------------------->*/\n"\
L"/*该文件由uiresbuilder生成，请不要手动修改*/\n"\
L"/*<------------------------------------------------------------------------------------------------->*/\n"
L"#define DEFINE_UIRES(name, type, file_path)\\\n"
L"    name type file_path\n\n";


struct IDMAPRECORD
{
	WCHAR szType[100];
	WCHAR szName[200];
	WCHAR szPath[MAX_PATH];
};

//获得文件的最后修改时间
__int64 GetLastWriteTime(LPCSTR pszFileName)
{
	__int64 tmFile=0;
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA(pszFileName, &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		tmFile= *(__int64*)&findFileData.ftLastWriteTime;
		FindClose(hFind);
	}
	return tmFile;
}

//将单反斜扛转换成双反斜扛
wstring BuildPath(LPCWSTR pszPath)
{
	LPCWSTR p=pszPath;
	WCHAR szBuf[MAX_PATH*2]={0};
	WCHAR *p2=szBuf;
	while(*p)
	{
		if(*p==L'\\')
		{
			if(*(p+1)!=L'\\')
			{//单斜扛
				p2[0]=p2[1]=L'\\';
				p++;
				p2+=2;
			}else
			{//已经是双斜扛
				p2[0]=p2[1]=L'\\';
				p+=2;
				p2+=2;
			}
		}else
		{
			*p2=*p;
			p++;
			p2++;
		}
	}
	*p2=0;
	return wstring(szBuf);
}

#define STAMP_FORMAT	L"//stamp:0000000000000000\r\n"
#define STAMP_FORMAT2	L"//stamp:%08x%08x\r\n"

#pragma pack(push,1)

class FILEHEAD
{
public:
	char szBom[2];
	WCHAR szHeadLine[ARRAYSIZE(STAMP_FORMAT)];

	FILEHEAD(__int64 ts=0)
	{
		szBom[0]=0xFF,szBom[1]=0xFE;
		swprintf(szHeadLine,STAMP_FORMAT2,(ULONG)((ts>>32)&0xffffffff),(ULONG)(ts&0xffffffff));		
	}
	static __int64 ExactTimeStamp(LPCSTR pszFile)
	{
		__int64 ts=0;
		FILE *f=fopen(pszFile,"rb");
		if(f)
		{
			FILEHEAD head;
			fread(&head,sizeof(FILEHEAD),1,f);
			DWORD dHi=0,dLow=0;
			if(wcsncmp(head.szHeadLine,STAMP_FORMAT2,8)==0)
			{
				swscanf(head.szHeadLine,STAMP_FORMAT2,&dHi,&dLow);
				ts=((__int64)dHi)<<32|dLow;
			}
			fclose(f);
		}
		return ts;
	}
};
#pragma  pack(pop)

//uiresbuilder -p skin -i skin\index.xml -r .\duires\winres.rc2
int _tmain(int argc, _TCHAR* argv[])
{
	string strSkinPath;	//皮肤路径,相对于程序的.rc文件
	string strIndexFile;
	string strRes;		//rc2文件名

	int c;

	printf("%s\n",GetCommandLineA());
	while ((c = getopt(argc, argv, _T("i:r:p:"))) != EOF)
	{
		switch (c)
		{
		case 'i':strIndexFile=optarg;break;
		case 'r':strRes=optarg;break;
		case 'p':strSkinPath=optarg;break;
		}
	}

	if(strIndexFile.empty())
	{
		printf("not specify input file, using -i to define the input file");
		return 1;
	}

	//打开index.xml文件
	TiXmlDocument xmlIndexFile;
	if(!xmlIndexFile.LoadFile(strIndexFile.c_str()))
	{
		printf("parse input file failed");
		return 1;
	}
    TiXmlElement *xmlResource=xmlIndexFile.FirstChildElement("resource");
    if(!xmlResource)
        {
        printf("invalid ui index file");
        return 2;
        }

	vector<IDMAPRECORD> vecIdMapRecord;
	//load xml description of resource to vector
    TiXmlElement *pXmlType=xmlResource->FirstChildElement();
	while(pXmlType)
	{
        WCHAR szType[100];
        const char *pszType=pXmlType->Value();
        MultiByteToWideChar(CP_UTF8,0,pszType,-1,szType,100);

        TiXmlElement *pXmlFile=pXmlType->FirstChildElement();
        while(pXmlFile)
            {
            IDMAPRECORD rec={0};
            wcscpy(rec.szType,szType);
            const char *pszValue;
            pszValue=pXmlFile->Attribute("name");
            if(pszValue) MultiByteToWideChar(CP_UTF8,0,pszValue,-1,rec.szName,200);
            pszValue=pXmlFile->Attribute("path");
            if(pszValue)
                {
                string str;
                if(!strSkinPath.empty()){ str=strSkinPath+"\\"+pszValue;}
                else str=pszValue;
                MultiByteToWideChar(CP_UTF8,0,str.c_str(),str.length(),rec.szPath,MAX_PATH);
                }

            vecIdMapRecord.push_back(rec);
            pXmlFile=pXmlFile->NextSiblingElement();
		}
		pXmlType=pXmlType->NextSiblingElement();
	}
	if(strRes.length())
	{//编译资源.rc2文件
		//build output string by wide char
		wstring strOut;

		vector<IDMAPRECORD>::iterator it2=vecIdMapRecord.begin();
		while(it2!=vecIdMapRecord.end())
		{
			WCHAR szRec[2000];
			wstring strPath=BuildPath(it2->szPath);
			swprintf(szRec,L"DEFINE_UIRES(%s,\t%s,\t%\"%s\")\n",it2->szName,it2->szType,strPath.c_str());
			strOut+=szRec;
			it2++;
		}

		__int64 tmIdx=GetLastWriteTime(strIndexFile.c_str());
		__int64 tmSave=FILEHEAD::ExactTimeStamp(strRes.c_str());
		//write output string to target res file
		if(tmIdx!=tmSave)
		{
			FILE * f=_tfopen(strRes.c_str(),_T("wb"));
			if(f)
			{
				FILEHEAD tmStamp(tmIdx);
				fwrite(&tmStamp,sizeof(FILEHEAD)-sizeof(WCHAR),1,f);//写UTF16文件头及时间。-sizeof(WCHAR)用来去除stamp最后一个\0
				fwrite(RB_HEADER,sizeof(WCHAR),wcslen(RB_HEADER),f);
				fwrite(strOut.c_str(),sizeof(WCHAR),strOut.length(),f);
				fclose(f);
				printf("build resource succeed!\n");
			}
		}else
		{
			printf("%s has not been modified\n",strIndexFile.c_str());
		}

	}
	return 0;
}


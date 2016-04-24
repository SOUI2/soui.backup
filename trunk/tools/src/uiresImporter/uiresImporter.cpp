// uiresIndexMake.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string  tstring;
#endif

void printUsage();
void ImportResource(xml_node xmlNode, const tstring & strUiresDir,const tstring & strNamePrefix,xml_node xmlSkin);

int _tmain(int argc, _TCHAR* argv[])
{
    tstring strUiresDir;    //uires路径，相对于当前路径
    tstring strSubDirs;     //搜索的子目录列表，子目录之间使用“|”分开.
    tstring strImgDir;      //图片目录，不要和子目录列表重复。imglist: file[3].png imgframe: file[3{2,2,2,2}].png
    bool    bBackup=true;
    int c;

    _tprintf(_T("%s\n"),GetCommandLine());

    while ((c = getopt(argc, argv, _T("p:s:i:b:"))) != EOF || optarg!=NULL)
    {
        switch (c)
        {
        case 'p':strUiresDir=optarg;break;
        case 's':strSubDirs=optarg;break;
        case 'i':strImgDir=optarg;break;
        case 'b': bBackup = _tcsicmp(optarg,_T("no"))==0?false:true;
        default: break;
        }
    }
    
    if(strUiresDir.empty() || strSubDirs.empty() || strImgDir.empty())
    {
        printUsage();
        return 1;
    }

    SetCurrentDirectory(strUiresDir.c_str());
    xml_document docUiIdx;
    if(!docUiIdx.load_file("uires.idx"))
    {
        printf("load uires.idx failed.");
        return 2;
    }
    
    vector<tstring> vecSubDirs;

    int iStart = 0;
    while(iStart<strSubDirs.size())
    {
        int iFind = strSubDirs.find(_T('|'),iStart);
        if(iFind == strSubDirs.npos)
            iFind = strSubDirs.size();
        tstring strSubDir = strSubDirs.substr(iStart,iFind - iStart);
        vecSubDirs.push_back(strSubDir);
        iStart = iFind + 1;
    }

    xml_node xmlRes = docUiIdx.child(_T("resource"));
    for(int i=0;i<vecSubDirs.size();i++)
    {
        xml_node xmlType = xmlRes.child(vecSubDirs[i].c_str());
        //make sure xmlType is an empty element
        if(xmlType) xmlRes.remove_child(xmlType);
        xmlType = xmlRes.append_child(vecSubDirs[i].c_str());
        
        ImportResource(xmlType,vecSubDirs[i].c_str(),_T(""),xml_node());
    }
    
    xml_node xmlImage = xmlRes.child(strImgDir.c_str());
    if(xmlImage) xmlRes.remove_child(xmlImage);
    xmlImage = xmlRes.append_child(strImgDir.c_str());

    xml_document docSkin;
    docSkin.load_file(L"values\\skin.xml");
    xml_node xmlSkin = docSkin.child(_T("skin"));
    if(!xmlSkin) xmlSkin = docSkin.append_child(_T("skin"));
    ImportResource(xmlImage,strImgDir,_T(""),xmlSkin);
    
    if(bBackup)
    {//备份数据
        ::CopyFile(_T("uires.idx"),_T("uires.bak.idx"),FALSE);
        ::CopyFile(_T("values\\skin.xml"),_T("values\\skin.bak.xml"),FALSE);
    }
    docUiIdx.save_file(L"uires.idx");
    docSkin.save_file(L"values\\skin.xml");
	return 0;
}

void printUsage()
{
    LPCTSTR  szUsage[] =
    {
        _T("UiresImporter Usage: uiresImporter -p path -s subdirs -i image -b backup\n"),
        _T("\tpath: specify uires path\n"),
        _T("\tsubdirs: specify subdirectorys that will be auto imported.\n"),
        _T("\timage: specify directory that will be auto imported as skin objects.\n"),
        _T("\t\timage name uses suffix format [states{left,top,right,bottom}] to identify skin format.\n"),
        _T("\tbackup: specify backup operating.\n"),
        _T("\t\t yes (defult) - backup\n"),
        _T("\t\t no - ignore backup\n"),
    };
    for(int i= 0;i< ARRAYSIZE(szUsage);i++)
    {
        _tprintf(szUsage[i]);
    }
}

void ImportResource(xml_node xmlNode, const tstring & strUiresDir,const tstring & strNamePrefix,xml_node xmlSkin)
{
    WIN32_FIND_DATA fd;
    HANDLE hContext = ::FindFirstFile((strUiresDir+_T("\\*.*")).c_str(), &fd);
    if(hContext!=INVALID_HANDLE_VALUE)
    {
        while(::FindNextFile(hContext,&fd))
        {
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if(_tcscmp(fd.cFileName,_T(".")) == 0 || _tcscmp(fd.cFileName,_T("..")) == 0)
                    continue;
                ImportResource(xmlNode, strUiresDir+_T("\\")+ fd.cFileName, strNamePrefix+ fd.cFileName + _T("."),xmlSkin);
            }else
            {
                xml_node newFile = xmlNode.append_child(_T("file"));
                TCHAR szName[MAX_PATH],szExt[50];
                _tsplitpath(fd.cFileName,NULL,NULL,szName,szExt);

                tstring strName,strPath;
                strName = strNamePrefix+szName;
                strPath = strUiresDir + _T("\\") + fd.cFileName;
                if(xmlSkin)
                {
                    LPTSTR p = _tcsrchr(szName,'[');
                    {
                        if(p) *p = 0;
                        strName = strNamePrefix+szName;
                        tstring src = xmlNode.name();
                        src += _T(":");
                        src += strName;
                        
                        xml_node node = xmlSkin.find_child_by_attribute(_T("name"),strName.c_str());
                        if(!node)
                        {//防止重复添加。
                            int nStates=1, left=-1,top=-1,right=-1,bottom=-1;
                            int nValues = !p?0:_stscanf(p+1,_T("%d{%d,%d,%d,%d}]"),&nStates,&left,&top,&right,&bottom);
                            if(nValues==0 || nValues == 1)
                            {//imglist
                                xml_node il = xmlSkin.append_child(_T("imglist"));
                                il.append_attribute(_T("name")).set_value(strName.c_str());
                                il.append_attribute(_T("src")).set_value(src.c_str());
                                il.append_attribute(_T("states")).set_value(nStates);
                            }else if(nValues==3 || nValues == 5)
                            {//imgframe
                                xml_node il = xmlSkin.append_child(_T("imgframe"));
                                il.append_attribute(_T("name")).set_value(strName.c_str());
                                il.append_attribute(_T("src")).set_value(src.c_str());
                                il.append_attribute(_T("states")).set_value(nStates);

                                TCHAR szMargin[100];
                                _stprintf(szMargin,_T("%d,%d,%d,%d"),left,top,right==-1?left:right,bottom==-1?top:bottom);
                                il.append_attribute(_T("margin")).set_value(szMargin);
                            }
                        }
                    }
                }
                newFile.append_attribute(L"name").set_value(strName.c_str());
                newFile.append_attribute(L"path").set_value(strPath.c_str());
            }
        }
        ::FindClose(hContext);
    }
}
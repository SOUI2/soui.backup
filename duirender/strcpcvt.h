#pragma  once

#ifndef CP_ACP
#define CP_ACP 0
#endif//CP_ACP

namespace SOUI
{

	class CDuiStrCpCvt
	{
	public:
		static CDuiStringA CvtW2A(const CDuiStringW & str,unsigned int cp=CP_ACP)
		{
			char szBuf[1024];
			int nRet=WideCharToMultiByte(cp,0,str,str.GetLength(),szBuf,1024,NULL,NULL);
			if(nRet>0) return CDuiStringA(szBuf,nRet);
			if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				int nRet=WideCharToMultiByte(cp,0,str,str.GetLength(),NULL,0,NULL,NULL);
				if(nRet>0)
				{
					char *pBuf=new char[nRet];
					WideCharToMultiByte(cp,0,str,str.GetLength(),pBuf,nRet,NULL,NULL);
					CDuiStringA strRet(pBuf,nRet);
					delete []pBuf;
					return strRet;
				}
			}
			return "";
		}

		static CDuiStringW CvtA2W(const CDuiStringA & str,unsigned int cp=CP_ACP,unsigned int cp2=0)
		{
			UNREFERENCED_PARAMETER(cp2);
			wchar_t szBuf[1024];
			int nRet=MultiByteToWideChar(cp,0,str,str.GetLength(),szBuf,1024);
			if(nRet>0)
			{
				return CDuiStringW(szBuf,nRet);
			}
			if(GetLastError()==ERROR_INSUFFICIENT_BUFFER)
			{
				int nRet=MultiByteToWideChar(cp,0,str,str.GetLength(),NULL,0);
				if(nRet>0)
				{
					wchar_t *pBuf=new wchar_t[nRet];
					MultiByteToWideChar(cp,0,str,str.GetLength(),pBuf,nRet);
					CDuiStringW strRet(pBuf,nRet);
					delete []pBuf;
					return strRet;
				}
			}
			return L"";
		}

		static CDuiStringA CvtA2A(const CDuiStringA & str,unsigned int cpFrom=CP_UTF8,unsigned int cpTo=CP_ACP)
		{
			if(cpTo==cpFrom)
				return str;
			CDuiStringW strw=CvtA2W(str,cpFrom);
			return CvtW2A(strw,cpTo);
		}

		static CDuiStringW CvtW2W(const CDuiStringW &str,unsigned int cp=CP_ACP)
		{
			return str;
		}

	};


}//end of namespace SOUI

#define DUI_CA2W CDuiStrCpCvt::CvtA2W
#define DUI_CW2A CDuiStrCpCvt::CvtW2A
#define DUI_CA2A CDuiStrCpCvt::CvtA2A
#define DUI_CW2W CDuiStrCpCvt::CvtW2W

#ifdef UNICODE
#define DUI_CA2T DUI_CA2W
#define DUI_CT2A DUI_CW2A
#define DUI_CW2T DUI_CW2W
#define DUI_CT2W DUI_CW2W
#else
#define DUI_CA2T DUI_CA2A
#define DUI_CT2A DUI_CA2A
#define DUI_CW2T DUI_CW2A
#define DUI_CT2W DUI_CA2W
#endif // UNICODE

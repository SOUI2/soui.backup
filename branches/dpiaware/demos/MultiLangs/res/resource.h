//stamp:091db7886008152e
/*<------------------------------------------------------------------------------------------------->*/
/*该文件由uiresbuilder生成，请不要手动修改*/
/*<------------------------------------------------------------------------------------------------->*/
#pragma once
#include <res.mgr/snamedvalue.h>
#define ROBJ_IN_CPP \
namespace SOUI \
{\
    const _R R;\
    const _UIRES UIRES;\
}
namespace SOUI
{
	class _UIRES{
		public:
		class _UIDEF{
			public:
			_UIDEF(){
				XML_INIT = _T("UIDEF:XML_INIT");
			}
			const TCHAR * XML_INIT;
		}UIDEF;
		class _LAYOUT{
			public:
			_LAYOUT(){
				XML_MAINWND = _T("LAYOUT:XML_MAINWND");
			}
			const TCHAR * XML_MAINWND;
		}LAYOUT;
		class _values{
			public:
			_values(){
				string = _T("values:string");
				color = _T("values:color");
				skin = _T("values:skin");
			}
			const TCHAR * string;
			const TCHAR * color;
			const TCHAR * skin;
		}values;
		class _lang{
			public:
			_lang(){
				cn = _T("lang:cn");
				en = _T("lang:en");
			}
			const TCHAR * cn;
			const TCHAR * en;
		}lang;
		class _ICON{
			public:
			_ICON(){
				ICON_LOGO = _T("ICON:ICON_LOGO");
			}
			const TCHAR * ICON_LOGO;
		}ICON;
		class _img{
			public:
			_img(){
				pic_100 = _T("img:pic_100");
				pic_125 = _T("img:pic_125");
				pic_150 = _T("img:pic_150");
				pic_200 = _T("img:pic_200");
			}
			const TCHAR * pic_100;
			const TCHAR * pic_125;
			const TCHAR * pic_150;
			const TCHAR * pic_200;
		}img;
		class _smenu{
			public:
			_smenu(){
				menu_lang = _T("smenu:menu_lang");
			}
			const TCHAR * menu_lang;
		}smenu;
	};
	const SNamedID::NAMEDVALUE namedXmlID[]={
		{L"btn_close",65540},
		{L"btn_max",65538},
		{L"btn_menu",65536},
		{L"btn_min",65537},
		{L"btn_restore",65539},
		{L"btn_scale_100",65541},
		{L"btn_scale_125",65542},
		{L"btn_scale_150",65543},
		{L"btn_scale_200",65544},
		{L"lang_cn",100},
		{L"lang_en",101},
		{L"txt_test",65545}		};
	class _R{
	public:
		class _name{
		public:
		_name(){
			btn_close = namedXmlID[0].strName;
			btn_max = namedXmlID[1].strName;
			btn_menu = namedXmlID[2].strName;
			btn_min = namedXmlID[3].strName;
			btn_restore = namedXmlID[4].strName;
			btn_scale_100 = namedXmlID[5].strName;
			btn_scale_125 = namedXmlID[6].strName;
			btn_scale_150 = namedXmlID[7].strName;
			btn_scale_200 = namedXmlID[8].strName;
			lang_cn = namedXmlID[9].strName;
			lang_en = namedXmlID[10].strName;
			txt_test = namedXmlID[11].strName;
		}
		 const wchar_t * btn_close;
		 const wchar_t * btn_max;
		 const wchar_t * btn_menu;
		 const wchar_t * btn_min;
		 const wchar_t * btn_restore;
		 const wchar_t * btn_scale_100;
		 const wchar_t * btn_scale_125;
		 const wchar_t * btn_scale_150;
		 const wchar_t * btn_scale_200;
		 const wchar_t * lang_cn;
		 const wchar_t * lang_en;
		 const wchar_t * txt_test;
		}name;

		class _id{
		public:
		const static int btn_close	=	65540;
		const static int btn_max	=	65538;
		const static int btn_menu	=	65536;
		const static int btn_min	=	65537;
		const static int btn_restore	=	65539;
		const static int btn_scale_100	=	65541;
		const static int btn_scale_125	=	65542;
		const static int btn_scale_150	=	65543;
		const static int btn_scale_200	=	65544;
		const static int lang_cn	=	100;
		const static int lang_en	=	101;
		const static int txt_test	=	65545;
		}id;

		class _string{
		public:
		const static int title	=	0;
		const static int ver	=	1;
		}string;

		class _color{
		public:
		const static int blue	=	0;
		const static int gray	=	1;
		const static int green	=	2;
		const static int red	=	3;
		const static int white	=	4;
		}color;

	};

#ifdef R_IN_CPP
	 extern const _R R;
	 extern const _UIRES UIRES;
#else
	 extern const __declspec(selectany) _R & R = _R();
	 extern const __declspec(selectany) _UIRES & UIRES = _UIRES();
#endif//R_IN_CPP
}

#pragma once

namespace SOUI
{

#define POSFLAG_REFCENTER    '|'        //参考父窗口中心
#define POSFLAG_REFPREV        '['        //参考前一个兄弟窗口
#define POSFLAG_REFNEXT        ']'        //参考下一个兄弟窗口
#define POSFLAG_PERCENT        '%'        //采用在父窗口的百分比定义坐标
#define POSFLAG_DEFSIZE        '@'        //在pos属性中定义窗口的size，只在在定义x2,y2时有效

    enum
    {
        POS_INIT=0x11000000,    //坐标的初始化值
        POS_WAIT=0x12000000,    //坐标的计算依赖于其它窗口的布局
    };

    enum //坐标属性
    {
        // Specify by "width" attribute
        SizeX_Mask          = 0x0007UL,
        SizeX_Specify       = 0x0001UL, // width > 0
        SizeX_FitContent    = 0x0002UL, // width <= 0
        SizeX_FitParent     = 0x0004UL, // width = "full" default

        // Specify by "height" attribute
        SizeY_Mask          = 0x0070UL,
        SizeY_Specify       = 0x0010UL, // height > 0
        SizeY_FitContent    = 0x0020UL, // height <= 0 default
        SizeY_FitParent     = 0x0040UL, // height = "full" default

        Position_Mask       = 0x0300UL, // 指定是浮动窗口还是有锚点窗口
        Pos_Float            = 0x0100UL,    // 1:浮动窗口，0:锚点窗口
    };

    //坐标类型
    enum PIT{
        PIT_NORMAL=0,    //一般坐标
        PIT_CENTER,        //参考父窗口中心点,以"|"开始
        PIT_PERCENT,    //指定在父窗口坐标的中的百分比,以"%"开头
        PIT_PREVSIBLING,    //指定坐标为相对前一个兄弟窗口的偏移，没有兄弟窗口时为父窗口,以"["开头
        PIT_NEXTSIBLING,    //指定坐标为相对前一个兄弟窗口的偏移，没有兄弟窗口时为父窗口,以"]"开头
        PIT_OFFSET,        //相对于前面x1,y1的偏移,只能在x2,y2中使用，以@开头
    };

    struct SWND_POSITION_ITEM
    {
        PIT pit;
        BOOL bMinus;
        float  nPos;
    };

    typedef enum tagPOS2TYPE{
        POS2_LEFTTOP=0,    //左上角
        POS2_RIGHTTOP,    //右上争
        POS2_CENTER,    //中心
        POS2_LEFTBOTTOM,//
        POS2_RIGHTBOTTOM,//
    }POS2TYPE;

    struct SWND_POSITION
    {
        int nCount;
        union
        {
            struct
            {
                SWND_POSITION_ITEM Left;
                SWND_POSITION_ITEM Top;
                SWND_POSITION_ITEM Right;
                SWND_POSITION_ITEM Bottom;
            };
            SWND_POSITION_ITEM Item[4];
        };
        UINT uPositionType;        //坐标属性
        POS2TYPE pos2Type;        //指定2点坐标时，坐标类型
        UINT uSpecifyWidth;
        UINT uSpecifyHeight;

        SWND_POSITION():nCount(0)
            ,uPositionType(0)
            ,pos2Type(POS2_LEFTTOP)
            ,uSpecifyWidth(0)
            ,uSpecifyHeight(0)
        {

        }
    };

    class SWindow;
    class SwndLayout
    {
    public:
        static void StrPos2SwndPos(LPCWSTR pszPos,SWND_POSITION &dlgpos);

        //************************************
        // Method:    PositionItem2Point ：将一个position_item解释为绝对坐标
        // FullName:  SOUI::SWindow::PositionItem2Point
        // Access:    protected 
        // Returns:   CPoint
        // Qualifier:
        // Parameter: const SWND_POSITION_ITEM & pos
        // Parameter: int nMin 父窗口的范围
        // Parameter: int nMax 父窗口的范围
        // Parameter: BOOL bX 计算X坐标
        //************************************
        static int PositionItem2Value(SWindow *pWnd,const SWND_POSITION_ITEM &pos,int nMin, int nMax,BOOL bX);

        //************************************
        // Method:    ParsePosition :解析一个坐标定义到position_item,增加对百分比的支持
        // FullName:  SOUI::SWindow::ParsePosition
        // Access:    protected 
        // Returns:   LPCSTR
        // Qualifier:
        // Parameter: LPCWSTR pszPos
        // Parameter: BOOL bFirst2Pos:TRUE-计算pos的前面两个值
        // Parameter: SWND_POSITION_ITEM & pos
        //************************************
        static LPCWSTR ParsePosition(LPCWSTR pszPos,BOOL bFirst2Pos,SWND_POSITION_ITEM &pos);

        //************************************
        // Method:    CalcPosition:计算窗口坐标
        // FullName:  SOUI::SWindow::CalcPosition
        // Access:    protected 
        // Returns:   int
        // Qualifier:
        // Parameter: LPRECT prcContainer
        //************************************
        static int CalcPosition(SWindow *pWnd,LPRECT prcContainer,const SWND_POSITION & dlgpos,CRect &rcWindow);


        static BOOL CalcChildrenPosition(SWindow *pWnd,SList<SWindow*> *pListChildren);
    };
}

/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SwndLayout.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    SOUI的窗口布局模块
*/

#pragma once

namespace SOUI
{

#define POSFLAG_REFCENTER      '|'        //参考父窗口中心
#define POSFLAG_REFPREV_NEAR   '['        //参考前一个兄弟窗口与自己近的边
#define POSFLAG_REFNEXT_NEAR   ']'        //参考下一个兄弟窗口与自己近的边
#define POSFLAG_REFPREV_FAR    '{'        //参考前一个兄弟窗口与自己远的边
#define POSFLAG_REFNEXT_FAR    '}'        //参考下一个兄弟窗口与自己远的边
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
        PIT_NORMAL=0,      //一般坐标
        PIT_CENTER,        //参考父窗口中心点,以"|"开始
        PIT_PERCENT,       //指定在父窗口坐标的中的百分比,以"%"开头
        PIT_PREV_NEAR,     //参考前一个兄弟窗口与自己近的边
        PIT_NEXT_NEAR,     //参考下一个兄弟窗口与自己近的边
        PIT_PREV_FAR,      //参考前一个兄弟窗口与自己远的边
        PIT_NEXT_FAR,      //参考下一个兄弟窗口与自己远的边
        PIT_OFFSET,        //相对于前面x1,y1的偏移,只能在x2,y2中使用，以@开头
    };

    struct POSITION_ITEM
    {
        PIT pit;
        BOOL bMinus;
        float  nPos;
    };

    typedef enum tagPOS2TYPE{
        POS2_LEFTTOP=0,    //左上角
        POS2_MIDTOP,
        POS2_RIGHTTOP,    //右上争
        POS2_LEFTMID,
        POS2_CENTER,    //中心
        POS2_RIGHTMID,
        POS2_LEFTBOTTOM,//左下角
        POS2_MIDBOTTOM,
        POS2_RIGHTBOTTOM,//右下角
    }POS2TYPE;

    class SWindow;
    class SOUI_EXP SwndLayout
    {
    public:
        SwndLayout(SWindow *pOwner);
        
        /**
         * ParseStrPostion
         * @brief    解析一个pos字符串
         * @param    LPCWSTR pszPos --  pos字符串
         * @return   void 
         *
         * Describe  
         */
        void ParseStrPostion(LPCWSTR pszPos);


        /**
         * CalcPosition
         * @brief    计算窗口坐标
         * @param    LPRECT prcContainer --  容器位置
         * @param  [out]  CRect & rcWindow --  窗口矩形
         * @return   int 计算得到的坐标个数
         *
         * Describe  每个窗口包含4个坐标，由于一个坐标可能依赖于其它兄弟窗口的布局，一次计算可能不能全部得到4个坐标
         */
        int CalcPosition(LPRECT prcContainer,CRect &rcWindow);


        /**
         * CalcChildrenPosition
         * @brief    计算列表中子窗口的坐标
         * @param    SList<SWindow * > * pListChildren --  子窗口列表
         * @return   BOOL TRUE-成功，FALSE-失败，可能由于布局依赖形成死锁
         *
         * Describe  
         */
        BOOL CalcChildrenPosition(SList<SWindow*> *pListChildren);
        
        /**
         * IsFitContent
         * @brief    获得布局是否依赖于内容标志
         * @return   BOOL 
         *
         * Describe  
         */
        BOOL IsFitContent();
        
        /**
         * IsFloat
         * @brief    查询窗口是否为Pos_float类型的窗口
         * @return   BOOL TRUE-float
         *
         * Describe  
         */
        BOOL IsFloat();
    protected:
    
        /**
         * PositionItem2Value
         * @brief    将一个position_item解释为绝对坐标
         * @param    const POSITION_ITEM & pos --  一个位置定义的引用
         * @param    int nMin --  父窗口的范围
         * @param    int nMax --  父窗口的范围
         * @param    BOOL bX --  计算X坐标
         * @return   int 计算得到的坐标
         *
         * Describe  
         */
        int PositionItem2Value(const POSITION_ITEM &pos,int nMin, int nMax,BOOL bX);

        /**
         * ParsePosition
         * @brief    解析一个字符串定义的坐标
         * @param    LPCWSTR pszPos --  "pos"属性字符串
         * @param    BOOL bFirst2Pos --  为前面两个坐标标志
         * @param  [out]  SWND_POSITION_ITEM & pos --  窗口坐标定义
         * @return   LPCWSTR 返回下一个位置开始
         *
         * Describe  前面两个坐标不能指定大小，后面的两个坐标才能指定大小
         */
        LPCWSTR ParsePosition(LPCWSTR pszPos,BOOL bFirst2Pos,POSITION_ITEM &pos);


        /**
         * CalcSize
         * @brief    计算窗口大小
         * @param    LPRECT pRcContainer --  容器位置
         * @return   CSize 
         *
         * Describe  
         */
        CSize CalcSize(LPRECT pRcContainer);

        SWindow *m_pOwner;  //**< layout的宿主 */
        
    public:
        int nCount;         //**< 定义的坐标个数 */
        union
        {
            struct
            {
                POSITION_ITEM Left;
                POSITION_ITEM Top;
                POSITION_ITEM Right;
                POSITION_ITEM Bottom;
            };
            POSITION_ITEM Item[4];
        };
        UINT uPositionType;       /**< 坐标属性 */
        POS2TYPE pos2Type;        /**< 指定2点坐标时，坐标类型 */
        UINT uSpecifyWidth;       /**< 指定的宽度 */
        UINT uSpecifyHeight;      /**< 指定的高度 */
        int  nSepSpace;           /**< 窗口水平自动排版的水平间隔，不支持垂直方向的自动排版 */
    };
}

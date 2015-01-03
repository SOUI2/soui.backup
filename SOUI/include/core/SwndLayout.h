/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SwndPosition.h
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
    #define POSFLAG_SIZE           '@'        //在pos属性中定义窗口的size，只在在定义x2,y2时有效

    enum SWNDPOSMASK//坐标属性
    {
        // Specify by "width" attribute
        SizeX_Mask          = 0x000fUL,
        SizeX_Specify       = 0x0001UL, // width > 0
        SizeX_FitContent    = 0x0002UL, // width <= 0
        SizeX_FitParent     = 0x0004UL, // width = "full" default

        // Specify by "height" attribute
        SizeY_Mask          = 0x00f0UL,
        SizeY_Specify       = 0x0010UL, // height > 0
        SizeY_FitContent    = 0x0020UL, // height <= 0 default
        SizeY_FitParent     = 0x0040UL, // height = "full" default
    };

    //坐标类型
    enum PIT{
        PIT_NULL=0,        //无效定义
        PIT_NORMAL,        //锚点坐标
        PIT_CENTER,        //参考父窗口中心点,以"|"开始
        PIT_PERCENT,       //指定在父窗口坐标的中的百分比
        PIT_PREV_NEAR,     //参考前一个兄弟窗口与自己近的边
        PIT_NEXT_NEAR,     //参考下一个兄弟窗口与自己近的边
        PIT_PREV_FAR,      //参考前一个兄弟窗口与自己远的边
        PIT_NEXT_FAR,      //参考下一个兄弟窗口与自己远的边
        PIT_SIZE,          //指定窗口的宽或者高
    };

    struct POSITION_ITEM
    {
        PIT     pit;
        char    cMinus;     /**<定义的值包含"-", 由于-0不能直接做nPos表示，需要一个单独的标志位 */
        float   nPos;       /**<坐标值 */
    };
  
    enum POSDIR
    {
        PD_X = 1,
        PD_Y = 2,
        PD_ALL = 3,
    };
    
    enum POSINDEX
    {
        PI_LEFT = 0,
        PI_TOP,
        PI_RIGHT,
        PI_BOTTOM,
    };

    class SOUI_EXP SwndLayout
    {
    public:
        SwndLayout();
        
        void Clear();
        
        BOOL IsEmpty();

        /**
         * InitPosFromString
         * @brief    解析一个pos字符串
         * @param    const SStringW & strPos --  pos字符串
         * @return   BOOL --  TRUE:成功，FALSE:失败 
         *
         * Describe  
         */
        BOOL InitPosFromString(const SStringW & strPos);
        
        /**
         * InitOffsetFromString
         * @brief    解析一个offset属性字符串
         * @param    const SStringW & strPos --  offset属性字符串
         * @return   BOOL --  TRUE:成功，FALSE:失败 
         * Describe  
         */    
        BOOL InitOffsetFromString(const SStringW & strPos);

         /**
         * InitOffsetFromPos2Type
         * @brief    从pos2type属性中初始化offset属性
         * @param    const SStringW & strPos2Type --  strPos2Type属性字符串
         * @return   BOOL --  TRUE:成功，FALSE:失败 
         * Describe  
         */    
        BOOL InitOffsetFromPos2Type(const SStringW & strPos2Type);

        //解析在size属性
        BOOL InitSizeFromString( const SStringW & strSize);

        BOOL InitWidth( const SStringW & strWidth);

        BOOL InitHeight(const SStringW & strHeight);

        BOOL SetFitParent(POSDIR pd = PD_ALL);
        
        BOOL SetFitContent(POSDIR pd = PD_ALL);
        
        BOOL IsFitParent(POSDIR pd = PD_ALL) const;

        /**
         * IsFitContent
         * @brief    获得布局是否依赖于内容标志
         * @return   BOOL 
         *
         * Describe  
         */
        BOOL IsFitContent(POSDIR pd = PD_ALL) const;
        
        BOOL IsSpecifySize(POSDIR pd = PD_ALL) const;

    protected:
        //将字符串描述的坐标转换成POSITION_ITEM
        BOOL StrPos2ItemPos(const SStringW &strPos,POSITION_ITEM & posItem);

        //解析在pos中定义的前两个位置
        BOOL ParsePosition12(const SStringW & pos1, const SStringW &pos2);

        //解析在pos中定义的后两个位置
        BOOL ParsePosition34(const SStringW & pos3, const SStringW &pos4);
        
        BOOL SetWidth(UINT nWid);

        BOOL SetHeight(UINT nHei);

    public:
        UINT uPositionType;         /**< 坐标属性 参见SWNDPOSMASK定义*/
        
        int  nCount;                /**< 定义的坐标个数 */
        POSITION_ITEM pos[4];       /**< 由pos属性定义的值, nCount >0 时有效*/
        UINT  uSpecifyWidth;        /**<使用width属性定义的宽 nCount==0 时有效*/
        UINT  uSpecifyHeight;       /**<使用height属性定义的高 nCount==0 时有效*/
        float fOffsetX,fOffsetY;    /**< 窗口坐标偏移量, x += fOffsetX * width, y += fOffsetY * height  */
    };
}

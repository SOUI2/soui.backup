#pragma once

namespace SOUI
{
class SOUI_EXP STileViewItemLocator : public TObjRefImpl<IObjRef>
{
public:
    STileViewItemLocator(int nItemHei, int nItemWid, int nMarginSize = 0);
    
    void SetAdapter(IAdapter *pAdapter);
    
    void OnDataSetChanged() {}
    
    int GetItemHeight(int iItem) const;
    void SetItemHeight(int iItem, int nHeight);
    
    //获取item的CRect(相对于TileView)
    CRect GetItemRect(int iItem);
    
    //设置TileView宽度（在TileView的OnSize中调用）
    void SetTileViewWidth(int width);
    
    //获取item的行、列位置
    void GetItemRowAndColIndex(int iItem, int &row, int &col);
    
    //是否为每行的最后一个元素
    BOOL IsLastInRow(int iItem);
    
    //获取上一行，同一列的元素index
    int GetUpItem(int iItem);
    //获取下一行，同一列的元素index
    int GetDownItem(int iItem);
    
    int GetTotalHeight();
    
    int Item2Position(int iItem);
    int Position2Item(int position);
    
    int GetScrollLineSize() const;
    
    int GetMarginSize() const
    {
        return m_nItemMargin;
    }
    
protected:
    //行高（包括间隔）
    int GetItemLineHeight() const
    {
        return m_nItemHeight + m_nItemMargin;
    }
    
    int m_nItemWidth;      //item宽
    int m_nItemHeight;     //item高
    int m_nTileViewWidth;  //TileView宽度（用于计算m_nCountInRow）
    int m_nItemMargin;     //块间距
    int m_nCountInRow;     //每行的item个数
    
    CAutoRefPtr<IAdapter> m_adapter;
};

}

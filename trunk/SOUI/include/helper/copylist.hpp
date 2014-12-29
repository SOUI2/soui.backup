#pragma  once

namespace SOUI
{
    /**
     * CopyList
     * @brief    列表Copy
     * @param    SList<T> & sour --  源List
     * @param    SList<T> & dest --  目标List
     * @return   void
     * Describe  
     */    
    template<class T>
    void CopyList(SList<T> &sour,SList<T> &dest)
    {
        SASSERT(dest.IsEmpty());
        SPOSITION pos=sour.GetHeadPosition();
        while(pos)
        {
            T &t=sour.GetNext(pos);
            dest.AddTail(t);
        }
    }

}
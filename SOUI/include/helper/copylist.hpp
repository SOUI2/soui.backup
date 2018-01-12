#pragma  once

namespace SOUI
{
    /**
     * CopyList
     * @brief    复制列表
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
	/**
	* ArrayFind
	* @brief    查找Array子项
	* @param    const SArray<T> & array --  源Array
	* @param     const T & obj --  目标项
	* @return   int -- 找到的项索引
	* @retval	-1--返回
	* Describe
	*/
    template<class T>
    int ArrayFind(const SArray<T> & array, const T & obj)
    {
        for(int i=0;i<(int)array.GetCount();i++)
        {
            if(array[i] == obj) return i;
        }
        return -1;
    }
}
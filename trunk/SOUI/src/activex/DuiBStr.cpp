#include "duistd.h"

#include "activex/DuiBStr.h"

namespace SOUI
{
        CDuiBStr::CDuiBStr(const char16* non_bstr)
            : bstr_(SysAllocString(non_bstr)) {}

        CDuiBStr::~CDuiBStr()
        {
            SysFreeString(bstr_);
        }

        void CDuiBStr::Reset(BSTR bstr)
        {
            if(bstr != bstr_)
            {
                // if |bstr_| is NULL, SysFreeString does nothing.
                SysFreeString(bstr_);
                bstr_ = bstr;
            }
        }

        BSTR CDuiBStr::Release()
        {
            BSTR bstr = bstr_;
            bstr_ = NULL;
            return bstr;
        }

        void CDuiBStr::Swap(CDuiBStr& bstr2)
        {
            BSTR tmp = bstr_;
            bstr_ = bstr2.bstr_;
            bstr2.bstr_ = tmp;
        }

        BSTR* CDuiBStr::Receive()
        {
            return &bstr_;
        }

        BSTR CDuiBStr::Allocate(const char16* str)
        {
            Reset(SysAllocString(str));
            return bstr_;
        }

        BSTR CDuiBStr::AllocateBytes(size_t bytes)
        {
            Reset(SysAllocStringByteLen(NULL, static_cast<UINT>(bytes)));
            return bstr_;
        }

        void CDuiBStr::SetByteLen(size_t bytes)
        {
            uint32* data = reinterpret_cast<uint32*>(bstr_);
            data[-1] = static_cast<uint32>(bytes);
        }

        size_t CDuiBStr::Length() const
        {
            return SysStringLen(bstr_);
        }

        size_t CDuiBStr::ByteLength() const
        {
            return SysStringByteLen(bstr_);
        }

} //namespace base
#ifndef __KEYVALUE_H___
#define __KEYVALUE_H___
#pragma once

#include <string>
#include <map>



namespace KEYVALUE
{
	template<typename ch, wchar_t cEqu='=', wchar_t cSep=';'>
	class kValueT
	{
	public:
		static bool get_value_bool(const ch* lpValue, bool def)
		{
			if (NULL == lpValue) return def;

			// only look at first char
			ch first = *lpValue;

			// 1*, t* (true), T* (True), y* (yes), Y* (YES)
			return (first == '1' || first == 't' || first == 'T' || first == 'y' || first == 'Y');
		}

		static int get_value_int(const ch* lpValue, int def)
		{
			if (NULL == lpValue) return def;

			return static_cast<unsigned int>(_tcstol(lpValue, 0, 10));
		}

		static unsigned int get_value_uint(const ch* lpValue, unsigned int def)
		{
			if (NULL == lpValue) return def;

			return static_cast<unsigned int>(_tcstoul(lpValue, 0, 10));
		}

		static long long get_value_llong(const ch* lpValue, long long def)
		{
			if (NULL == lpValue) return def;

			return static_cast<long long>(_tcstoi64(lpValue, 0, 10));
		}

		static unsigned long long get_value_ullong(const ch* lpValue, unsigned long long def)
		{
			if (NULL == lpValue) return def;

			return static_cast<unsigned long long>(_tcstoui64(lpValue, 0, 10));
		}

		static double get_value_double(const ch* lpValue, double def)
		{
			if (NULL == lpValue) return def;

			return _tcstod(lpValue, 0);
		}

		static float get_value_float(const ch* lpValue, float def)
		{
			if (NULL == lpValue) return def;

			return static_cast<float>(_tcstod(lpValue, 0));
		}
	public:
		kValueT()
		{

		}
		bool Parse(const ch* lpStr)
		{
			int nLen = lstrlenW(lpStr);
			
			int nKeyBegin = 0;
			int nKeyEnd = 0;
			int nValueBegin = 0;
	
			for (int i=1; i<=nLen; ++i)
			{
				if(cEqu == lpStr[i])			// 有 等于 
				{
					nKeyEnd = i;
					nValueBegin = i+1;
				}
				else if(cSep == lpStr[i] || '\0' == lpStr[i])
				{
					std::basic_string<ch> sKey(lpStr+nKeyBegin, nKeyEnd - nKeyBegin);
					std::basic_string<ch> sValue(lpStr+nValueBegin, i - nValueBegin);

					m_mapValue[sKey] = sValue;

					nKeyBegin = i + 1;
					nKeyEnd = nKeyBegin;
					nValueBegin = nKeyBegin;
				}
			}

			return true;
		}
		// 取 key 对应的 string值
		const wchar_t* GetStringValue(const ch* lpKey, const ch* def=NULL)
		{
			const ch* lp = get_value(lpKey);
			
			return (NULL == lp) ? def : lp;
		}
		// 取 key 对应的 bool值
		bool GetBoolValue(const ch* lpKey, bool def=false)
		{
			return get_value_bool(get_value(lpKey), def);
		}
		// 取 key 对应的 int值
		int GetIntValue(const ch* lpKey, int def=0)
		{
			return get_value_int(get_value(lpKey), def);
		}
		unsigned int GetUintValue(const ch* lpKey, unsigned int def=0)
		{
			return get_value_uint(get_value(lpKey), def);
		}
		long long GetInt64Value(const ch* lpKey, long long def=0)
		{
			return get_value_llong(get_value(lpKey), def);
		}
		unsigned long long GetUint64Value(const ch* lpKey, unsigned long long def=0)
		{
			return get_value_ullong(get_value(lpKey), def);
		}
		float GetFloatValue(const ch* lpKey, float def=0)
		{
			return get_value_float(get_value(lpKey), def);
		}
		double GetDoubleValue(const ch* lpKey, double def=0)
		{
			return get_value_double(get_value(lpKey), def);
		}
	private:
		inline const ch* get_value(const ch* lpKey)
		{
			auto ite = m_mapValue.find(lpKey);
			if(ite == m_mapValue.end())
			{
				return NULL;
			}
			return ite->second.c_str();
		}
	private:
		std::map<std::basic_string<ch>, std::basic_string<ch>>		m_mapValue;
	};

}




typedef KEYVALUE::kValueT<char, '=', ';'>				kValueA;
typedef KEYVALUE::kValueT<wchar_t, '=', ';'>		kValueW;

#ifdef _UNICODE
	typedef kValueW                        kValue;
#else
	typedef kValueA                        kValue;
#endif

#endif
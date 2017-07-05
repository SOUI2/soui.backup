#include "stdafx.h"
#include "pugixml_write.h"


myxml_writer_stream::myxml_writer_stream(SStringA& stream) : narrow_stream(&stream), wide_stream(0)
{
}

myxml_writer_stream::myxml_writer_stream(SStringW& stream) : narrow_stream(0), wide_stream(&stream)
{
}

void myxml_writer_stream::write(const void* data, size_t size)
{
	if (narrow_stream)
	{
		assert(!wide_stream);
		char* buf = new char[size + 1];
		strncpy(buf, reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
		buf[size] = 0;
		narrow_stream->Append(buf);
		delete[] buf;
	}
	else
	{
		assert(wide_stream);
		assert(size % sizeof(wchar_t) == 0);
		wchar_t* buf = new wchar_t[size + 1];
		wcsncpy(buf, reinterpret_cast<const wchar_t*>(data), static_cast<std::streamsize>(size / sizeof(wchar_t)));
		buf[size / sizeof(wchar_t)] = 0;
		wide_stream->Append(buf);
		delete[] buf;
	}
}
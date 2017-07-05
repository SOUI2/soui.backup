#pragma once


// xml_writer implementation for streams
class myxml_writer_stream : public pugi::xml_writer
{
public:
	// Construct writer from an output stream object
	myxml_writer_stream(SStringA& stream);
	myxml_writer_stream(SStringW& stream);

	virtual void write(const void* data, size_t size);

private:
	SStringA* narrow_stream;
	SStringW* wide_stream;
};
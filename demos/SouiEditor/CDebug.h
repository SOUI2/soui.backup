
class CDebug
{
public:
	CDebug(){};
	~CDebug(){};

	static void Debug(pugi::xml_node xmlNode)
	{
		pugi::xml_writer_buff writer;
		xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
		SMessageBox(NULL, *strDebug, _T("提示"), MB_OK);
		delete strDebug;
	};

	static void Debug(SStringT str)
	{
	    SMessageBox(NULL, str, _T("提示"), MB_OK);
	};
	static SStringT Debug1(pugi::xml_node xmlNode)
	{
		pugi::xml_writer_buff writer;
		xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
		SStringT strtemp = *strDebug;
		delete strDebug;
		return strtemp;
	};

private:

};
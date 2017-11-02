#pragma once
class CRGBA2ICON
{
private:
	HANDLE m_hFile;
	SArray<IBitmap*> m_pBitmapList;
	bool createFile(LPCTSTR pszFileName)
	{
		m_hFile = ::CreateFile(pszFileName, GENERIC_ALL, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == m_hFile)
			return false;
		return true;
	}
	bool writeByte(LPCVOID data, DWORD dataSize)
	{
		return WriteFile(m_hFile, data, dataSize, NULL, NULL) == TRUE;
	}
	bool writeByte(unsigned int val, int byte)
	{
		char data[4];
		assert(byte <= 4);
		memcpy((void *)data, (void *)&val, byte);
		return writeByte(data, byte);
	};
	bool createIconFile(LPCTSTR pszFileName)
	{
		if (createFile(pszFileName))
		{
			int x, y;
			int index = 0;
			int Size = 0;
			int offset = 6 + m_pBitmapList.GetCount() * 16;
			int bpp = 32;
			writeByte(0, 2);				//idReserved
			writeByte(1, 2);				//idType 1为图标 2为光标
			writeByte(m_pBitmapList.GetCount(), 2);//idCount 图像数量
			//
			int width = 0, height = 0;
			//字入所有的头
			for (int i = 0; i < m_pBitmapList.GetCount(); i++)
			{
				CSize size = m_pBitmapList[i]->Size();
				width = size.cx;
				height = size.cy;
				//图像块16字节
				writeByte(width, 1);			//bWidth
				writeByte(height, 1);			//bHeight
				writeByte(0, 1);				//bColorCount
				writeByte(0, 1);				//bReserved
				writeByte(1, 2);				//wPlanes
				writeByte(bpp, 2);				//wBitCount
				Size = 40 + height * ((width + 31) / 32 * 32 / 8 + width * 3);	//Note 4 bytes alignment
				Size += height * width;//AND MASK
				writeByte(Size, 4);			//dwBytesInRes
				writeByte(offset, 4);		//dwImageOffset
				offset += Size;
			}
			//写入实际的数据 
			for (int i = 0; i < m_pBitmapList.GetCount(); i++)
			{
				CSize size = m_pBitmapList[i]->Size();
				width = size.cx;
				height = size.cy;				
				//BMP头40字节
				writeByte(40, 4);				//biSize
				writeByte(width, 4);			//biWidth
				writeByte(2 * height, 4);		//biHeight 
				writeByte(1, 2);				//biPlanes
				writeByte(bpp, 2);				//biBitCount
				writeByte(0, 4);				//biCompression
				writeByte(0, 4);				//biSizeImage
				writeByte(0, 4);				//biXPelsPerMeter   
				writeByte(0, 4);				//biYPelsPerMeter
				writeByte(0, 4);				//biClrUsed 
				writeByte(0, 4);				//biClrImportant

				unsigned char * data = (unsigned char *)m_pBitmapList[i]->LockPixelBits();
				// XOR mask
				for (y = height - 1; y >= 0; --y) {
					for (x = 0; x < width; ++x) {
						index = (y * width + x) * 4;
						writeByte(data[index], 1);        //Blue 
						writeByte(data[index + 1], 1);    //Green
						writeByte(data[index + 2], 1);    //Red
						writeByte(data[index + 3], 1);    //Alpha
					}
				}
				// AND mask
				for (y = 0; y < (height * ((width + 31) / 32 * 32 / 8)); ++y) {
					writeByte(0, 1);
				}
			}
			CloseHandle(m_hFile);
			return true;
		}
		return false;
	}
public:
	bool SaveIconFile(LPCTSTR pszFileName)
	{
		return createIconFile(pszFileName);
	}
	void AddBitmapToIco(IBitmap *pBitmap)
	{
		m_pBitmapList.Add(pBitmap);
	}
};
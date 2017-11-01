#pragma once
class CRGBA2ICON
{
private:
	HANDLE m_hFile;
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
	bool createIconFile(LPCTSTR pszFileName, int width, int height, unsigned char* data)
	{
		if (createFile(pszFileName))
		{
			int x, y;
			int index = 0;
			int Size = 0;
			int offset = 6 + 1 * 16;
			int bpp = 32;
			writeByte(0, 2);				//idReserved
			writeByte(1, 2);				//idType
			writeByte(1, 2);				//idCount 

			writeByte(width, 1);			//bWidth
			writeByte(height, 1);			//bHeight
			writeByte(0, 1);				//bColorCount
			writeByte(0, 1);				//bReserved
			writeByte(1, 2);				//wPlanes
			writeByte(bpp, 2);				//wBitCount
			Size = 40 + height * ((width + 31) / 32 * 32 / 8 + width * 3);	//Note 4 bytes alignment
			if (bpp == 32)
				Size += height * width;
			writeByte(Size, 4);			//dwBytesInRes
			writeByte(offset, 4);			//dwImageOffset

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

			// XOR mask
			for (y = height - 1; y >= 0; --y) {
				for (x = 0; x < width; ++x) {
					index = (y * width + x) * 4;
					writeByte(data[index], 1);        //Blue 
					writeByte(data[index + 1], 1);    //Green
					writeByte(data[index + 2], 1);    //Red
					writeByte(data[index + 3], 1); //Alpha
				}
			}

			// AND mask
			for (y = 0; y < (height * ((width + 31) / 32 * 32 / 8)); ++y) {
				writeByte(0, 1);
			}
			CloseHandle(m_hFile);
			return true;
		}
		return false;
	}
public:
	bool createIconFile(LPCTSTR pszFileName, int width, int height, IBitmap * pBmp)
	{
		LPBYTE pData = (LPBYTE)pBmp->LockPixelBits();
		return createIconFile(pszFileName, width, height, pData);
	}
};
#pragma once
#include <_File.h>

//Only support 24bit bmp!
struct BMP
{
#pragma pack(1)//自定义字节对齐方式，连续存储
	struct Header
	{
		char identifier[2];
		unsigned int fileSize;
		unsigned int reserved;
		unsigned int dataOffset;
		unsigned int headerSize;//本结构占据的字节数
		unsigned int width;
		unsigned int height;
		unsigned short planeNum;
		unsigned short bitsPerPixel;
		unsigned int compressionMethod;
		unsigned int dataSize;//位图的大小
		unsigned int verticalPixelsPerMeter;//水平分辨率
		unsigned int horizontalPixelsPerMeter;//垂直分辨率
		unsigned int colorNum;
		unsigned int importantColorNum;
		void printInfo()const
		{
			::printf("BMP file info:\n");
			::printf("\tFile Size:\t%u\n", fileSize);
			::printf("\tWidth:\t\t%u\n", width);
			::printf("\tHeight:\t\t%u\n", height);
		}
	};
	struct Pixel
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
	};
#pragma pack()//取消自定义字节的对齐方式

	Header header;
	Pixel* data;

	BMP()
		:
		header(),
		data(nullptr)
	{
	}
	BMP(String<char>const& _path)
	{
		FILE* temp(::fopen(_path.data, "rb+"));
		::fseek(temp, 0, SEEK_SET);//把temp有关的文件位置指针放到文件的开头
		::fread(&header, 1, 54, temp);//把BMP文件的文件头读取
		if (header.width % 4)//补齐
			data = (BMP::Pixel*)::malloc(3u * header.width * header.height + 4);
		else
			data = (BMP::Pixel*)::malloc(3u * header.width * header.height);
		for (int c0(0); c0 < header.height; ++c0)
			::fread((data + header.width * c0), 4, header.width * 3/4, temp);
		::fclose(temp);
	}
	~BMP()
	{
		::free(data);
	}

	bool checkType()const
	{
		return header.identifier[0] == 'B' && header.identifier[1] == 'M';
	}
	void printInfo()const
	{
		header.printInfo();
	}
};

//File...
inline BMP File::readBMP()const
{
	if (!this)return BMP();
	BMP r;
	FILE* temp(::fopen((property.path + property.file.name).data, "rb+"));
	::fseek(temp, 0, SEEK_SET);
	::fread(&r.header, 1, 54, temp);
	if (r.header.width % 4)
		r.data = (BMP::Pixel*)::malloc(3u * r.header.width * r.header.height + 4);
	else
		r.data = (BMP::Pixel*)::malloc(3u * r.header.width * r.header.height);
	for (int c0(0); c0 < r.header.height; ++c0)
		::fread((r.data + r.header.width), 4, r.header.width, temp);
	::fclose(temp);
	return r;
}
inline BMP File::readBMP(String<char> const& _name)const
{
	if (!this)return BMP();
	BMP r;
	FILE* temp(::fopen((property.path + _name).data, "rb+"));
	::fseek(temp, 0, SEEK_SET);
	::fread(&r.header, 1, 54, temp);
	if (r.header.width % 4)
		r.data = (BMP::Pixel*)::malloc(3u * r.header.width * r.header.height + 4);
	else
		r.data = (BMP::Pixel*)::malloc(3u * r.header.width * r.header.height);
	for (int c0(0); c0 < r.header.height; ++c0)
		::fread((r.data + r.header.width), 4, r.header.width, temp);
	::fclose(temp);
	return r;
}
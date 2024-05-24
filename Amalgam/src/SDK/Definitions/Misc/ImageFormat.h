#pragma once

enum NormalDecodeMode_t
{
	NORMAL_DECODE_NONE = 0
};

//typedef enum _D3DFORMAT D3DFORMAT;

enum ImageFormat
{
	IMAGE_FORMAT_UNKNOWN = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGB888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565,
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888,
	IMAGE_FORMAT_R32F,
	IMAGE_FORMAT_RGB323232F,
	IMAGE_FORMAT_RGBA32323232F,
	IMAGE_FORMAT_NV_DST16,
	IMAGE_FORMAT_NV_DST24,
	IMAGE_FORMAT_NV_INTZ,
	IMAGE_FORMAT_NV_RAWZ,
	IMAGE_FORMAT_ATI_DST16,
	IMAGE_FORMAT_ATI_DST24,
	IMAGE_FORMAT_NV_NULL,
	IMAGE_FORMAT_ATI2N,
	IMAGE_FORMAT_ATI1N,
	IMAGE_FORMAT_DXT1_RUNTIME,
	IMAGE_FORMAT_DXT5_RUNTIME,
	NUM_IMAGE_FORMATS
};

struct BGRA8888_t
{
	unsigned char b = 0;
	unsigned char g = 0;
	unsigned char r = 0;
	unsigned char a = 0;

	inline BGRA8888_t& operator=(const BGRA8888_t& in)
	{
		*(unsigned int*)this = *(unsigned int*)&in;
		return *this;
	}
};

struct BGRX8888_t
{
	unsigned char b = 0;
	unsigned char g = 0;
	unsigned char r = 0;
	unsigned char x = 0;

	inline BGRX8888_t& operator=(const BGRX8888_t& in)
	{
		*(unsigned int*)this = *(unsigned int*)&in;
		return *this;
	}
};

struct RGBA8888_t
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;
	unsigned char a = 0;
};

struct RGB888_t
{
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	inline RGB888_t& operator=(const BGRA8888_t& in)
	{
		r = in.r;
		g = in.g;
		b = in.b;
		return *this;
	}

	inline bool operator==(const RGB888_t& in) const
	{
		return (r == in.r) && (g == in.g) && (b == in.b);
	}

	inline bool operator!=(const RGB888_t& in) const
	{
		return (r != in.r) || (g != in.g) || (b != in.b);
	}
};

struct BGR888_t
{
	unsigned char b = 0;
	unsigned char g = 0;
	unsigned char r = 0;

	inline BGR888_t& operator=(const BGRA8888_t& in)
	{
		r = in.r;
		g = in.g;
		b = in.b;
		return *this;
	}
};

struct BGR565_t
{
	unsigned short b : 5;
	unsigned short g : 6;
	unsigned short r : 5;

	inline BGR565_t& operator=(const BGRA8888_t& in)
	{
		r = in.r >> 3;
		g = in.g >> 2;
		b = in.b >> 3;
		return *this;
	}

	inline BGR565_t& Set(int red, int green, int blue)
	{
		r = red >> 3;
		g = green >> 2;
		b = blue >> 3;
		return *this;
	}
};

struct BGRA5551_t
{
	unsigned short b : 5;
	unsigned short g : 5;
	unsigned short r : 5;
	unsigned short a : 1;

	inline BGRA5551_t& operator=(const BGRA8888_t& in)
	{
		r = in.r >> 3;
		g = in.g >> 3;
		b = in.b >> 3;
		a = in.a >> 7;
		return *this;
	}
};

struct BGRA4444_t
{
	unsigned short b : 4;
	unsigned short g : 4;
	unsigned short r : 4;
	unsigned short a : 4;

	inline BGRA4444_t& operator=(const BGRA8888_t& in)
	{
		r = in.r >> 4;
		g = in.g >> 4;
		b = in.b >> 4;
		a = in.a >> 4;
		return *this;
	}
};

struct RGBX5551_t
{
	unsigned short r : 5;
	unsigned short g : 5;
	unsigned short b : 5;
	unsigned short x : 1;

	inline RGBX5551_t& operator=(const BGRA8888_t& in)
	{
		r = in.r >> 3;
		g = in.g >> 3;
		b = in.b >> 3;
		return *this;
	}
};
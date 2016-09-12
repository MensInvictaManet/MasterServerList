#pragma once

#include <Winsock2.h>
#include <algorithm>

class SocketBuffer
{
	static char retval[20001];
public:
	char* data;
	int BuffSize;
	int readpos;
	int writepos;
	void StreamWrite(void *in, int size);
	void StreamRead(void* out, int size, bool peek);
	int count;
	SocketBuffer();
	~SocketBuffer();
	int 				writechar(unsigned char a);
	int 				writeshort(short a);
	int 				writeushort(unsigned short a);
	int 				writeint(int a);
	int 				writeuint(unsigned int a);
	int 				writefloat(float a);
	int 				writedouble(double a);
	int 				writechars(char*str);
	int 				writechars(const char*str);
	int 				writestring(char*str);
	int 				writestring(const char*str);

	unsigned char	readchar(bool peek = false);
	short				readshort(bool peek = false);
	unsigned short	readushort(bool peek = false);
	int				readint(bool peek = false);
	unsigned int	readuint(bool peek = false);
	float				readfloat(bool peek = false);
	double			readdouble(bool peek = false);
	char*				readchars(int len, bool peek = false);
	char*				readstring(bool peek = false);

	int bytesleft() const;
	void StreamSet(int pos);
	void clear();
	int addBuffer(char*, int);
	int addBuffer(SocketBuffer*);
	char operator[](int index) const;
};

#define SIZEOF_CHAR sizeof(char)
#define SIZEOF_SHRT sizeof(short)
#define SIZEOF_USRT sizeof(unsigned short)
#define SIZEOF_INTE sizeof(int)
#define SIZEOF_UINT sizeof(unsigned int)
#define SIZEOF_FLOT sizeof(float)
#define SIZEOF_DOUB sizeof(double)

char SocketBuffer::retval[20001];

inline SocketBuffer::SocketBuffer()
{
	BuffSize = 30;
	data = static_cast<char*>(malloc(BuffSize));
	count = 0;
	readpos = 0;
	writepos = 0;
}

inline SocketBuffer::~SocketBuffer()
{
	if (data != nullptr) free(data);
}

inline void SocketBuffer::StreamWrite(void *in, int size)
{
	if (writepos + size >= BuffSize)
	{
		BuffSize = writepos + size + 30;
		if ((data = static_cast<char*>(realloc(data, BuffSize))) == nullptr) return;
	}
	memcpy(data + writepos, in, size);
	writepos += size;
	if (writepos > count) count = writepos;
}

inline void SocketBuffer::StreamRead(void* out, int size, bool peek)
{
	if (readpos + size > count) size = count - readpos;
	if (size <= 0) return;
	memcpy(out, data + readpos, size);

	if (!peek)
	{
		readpos += size;
	}
}

inline int SocketBuffer::writechar(unsigned char val)
{
	StreamWrite(&val, 1);
	return SIZEOF_CHAR;
}

inline int SocketBuffer::writeshort(short a)
{
	StreamWrite(&a, 2);
	return SIZEOF_SHRT;
}

inline int SocketBuffer::writeushort(unsigned short a)
{
	StreamWrite(&a, 2);
	return SIZEOF_USRT;
}
inline int SocketBuffer::writeint(int a)
{
	StreamWrite(&a, 4);
	return SIZEOF_INTE;
}
inline int SocketBuffer::writeuint(unsigned int a)
{
	StreamWrite(&a, 4);
	return SIZEOF_UINT;
}

inline int SocketBuffer::writefloat(float a)
{
	StreamWrite(&a, 4);
	return SIZEOF_FLOT;
}
inline int SocketBuffer::writedouble(double a)
{
	StreamWrite(&a, 8);
	return SIZEOF_DOUB;
}

inline int SocketBuffer::writechars(char*str)
{
	auto len = int(strlen(str));
	StreamWrite(str, len);
	return len;
}

inline int SocketBuffer::writechars(const char*str)
{
	int len = int(strlen(str));
	StreamWrite((void*)(str), len);
	return len;
}

inline int SocketBuffer::writestring(char *str)
{
	auto len = writechars(str);
	return len + writechar('\0');
}

inline int SocketBuffer::writestring(const char *str)
{
	auto len = writechars(str);
	return len + writechar('\0');
}

inline unsigned char SocketBuffer::readchar(bool peek)
{
	unsigned char b;
	StreamRead(&b, 1, peek);
	return b;
}

inline short SocketBuffer::readshort(bool peek)
{
	short b;
	StreamRead(&b, 2, peek);
	return b;
}

inline unsigned short SocketBuffer::readushort(bool peek)
{
	unsigned short b;
	StreamRead(&b, 2, peek);
	return b;
}

inline int SocketBuffer::readint(bool peek)
{
	int b;
	StreamRead(&b, 4, peek);
	return b;
}

inline unsigned int SocketBuffer::readuint(bool peek)
{
	unsigned int b;
	StreamRead(&b, 4, peek);
	return b;
}

inline float SocketBuffer::readfloat(bool peek)
{
	float b;
	StreamRead(&b, 4, peek);
	return b;
}
inline double SocketBuffer::readdouble(bool peek)
{
	double b;
	StreamRead(&b, 8, peek);
	return b;
}

inline char* SocketBuffer::readchars(int len, bool peek)
{

	if (len < 0) return nullptr;
	StreamRead(&retval, len, peek);
	retval[len] = '\0';
	return retval;
}

inline char* SocketBuffer::readstring(bool peek)
{
	int i;
	for (i = readpos; i < count; i++)
		if (data[i] == '\0') break;

	if (i == count) return nullptr;
	i -= readpos;
	i = std::min(20000, i);
	StreamRead(&retval, i + 1, peek);
	return retval;
}

inline int SocketBuffer::bytesleft() const
{
	return count - readpos;
}

inline int SocketBuffer::addBuffer(SocketBuffer *buffer)
{
	StreamWrite(buffer->data, buffer->count);
	return buffer->count;
}

inline int SocketBuffer::addBuffer(char *data, int len)
{
	StreamWrite(data, len);
	return len;
}

inline void SocketBuffer::clear()
{
	if (BuffSize > 30)
	{
		free(data);
		BuffSize = 30;
		data = static_cast<char*>(malloc(BuffSize));
	}
	count = 0;
	readpos = 0;
	writepos = 0;
}

inline void SocketBuffer::StreamSet(int pos)
{
	readpos = 0;
	writepos = 0;
}

inline char SocketBuffer::operator [](int i) const
{
	return ((i < 0 || i >= count) ? '\0' : data[i]);
}
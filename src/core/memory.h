#pragma once

#include "common.h"

LUMINA_NAMESPACE_BEGIN

#ifndef LUMINA_L1_CACHE_SIZE
#define LUMINA_L1_CACHE_SIZE 64
#endif

void* AllocAligned(size_t size);
template <typename T>
T* AllocAligned(size_t count) {
	size_t finalSize = count * sizeof(T);

	std::cout << sizeof(T) << "\n";
	std::cout << count << "\n";
	std::cout << finalSize << "\n";
	return (T*)AllocAligned(count * sizeof(T));
}

void FreeAligned(void*);

template <typename T, int logBlockSize = 2>
class BlockedArray {
public:
	BlockedArray(int uRes, int vRes, const T* d = nullptr);
	~BlockedArray();

	int blockSize() const;
	int roundUp(int x) const;

	int uSize() const { return uRes; }
	int vSize() const { return vRes; }
	int block(int a) const;
	int offset(int a) const;

	T& operator()(int u, int v);

	const T& operator()(int u, int v) const;

	void getLinearArray(T* a) const;

private:
	T* data;
	const int uRes, vRes, uBlocks;
};

template<typename T, int logBlockSize>
inline BlockedArray<T, logBlockSize>::BlockedArray(int uRes, int vRes, const T* d)
	: uRes(uRes), vRes(vRes), uBlocks(roundUp(uRes) >> logBlockSize)
{
	int nAlloc = roundUp(uRes) * roundUp(vRes);
	data = AllocAligned<T>(nAlloc);

	for (int i = 0; i < nAlloc; i++)
		new (&data[i]) T();

	if (d) {
		for (int v = 0; v < vRes; v++)
			for (int u = 0; u < uRes; u++)
				(*this)(u, v) = d[v * uRes + u];
	}
}

template<typename T, int logBlockSize>
BlockedArray<T, logBlockSize>::~BlockedArray()
{
	for (int i = 0; i < uRes * vRes; i++)
		data[i].~T();

	FreeAligned(data);
}

template<typename T, int logBlockSize>
int BlockedArray<T, logBlockSize>::blockSize() const
{
	return 1 << logBlockSize;
}

template<typename T, int logBlockSize>
int BlockedArray<T, logBlockSize>::roundUp(int x) const
{
	return (x + blockSize() - 1) & ~(blockSize() - 1);
}

template<typename T, int logBlockSize>
int BlockedArray<T, logBlockSize>::block(int a) const
{
	return a >> logBlockSize;
}

template<typename T, int logBlockSize>
int BlockedArray<T, logBlockSize>::offset(int a) const
{
	return (a & (blockSize() - 1));
}

template<typename T, int logBlockSize>
T& BlockedArray<T, logBlockSize>::operator()(int u, int v)
{
	int bu = block(u), bv = block(v);
	int ou = offset(u), ov = offset(v);
	int offset = blockSize() * blockSize() * (uBlocks * bv + bu);
	offset += blockSize() * ov + ou;

	return data[offset];
}

template<typename T, int logBlockSize>
const T& BlockedArray<T, logBlockSize>::operator()(int u, int v) const
{
	int bu = block(u), bv = block(v);
	int ou = offset(u), ov = offset(v);
	int offset = blockSize() * blockSize() * (uBloxk * bv + bu);
	offset += blockSize() * ov + ou;

	return data[offset];
}

template<typename T, int logBlockSize>
void BlockedArray<T, logBlockSize>::getLinearArray(T* a) const
{
	for (int v = 0; v < vRes; v++)
		for (int u = 0; u < uRes; u++)
			*a++ = (*this)(u, v);
}

LUMINA_NAMESPACE_END


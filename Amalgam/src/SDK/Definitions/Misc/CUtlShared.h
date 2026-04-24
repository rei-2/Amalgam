#pragma once

#define COMPILE_TIME_ASSERT( pred ) static_assert( pred, "Compile time assert constraint is not true: " #pred )

template <class T>
inline void Construct(T* pMemory)
{
	::new(pMemory) T;
}

template <class T>
inline void CopyConstruct(T* pMemory, T const& src)
{
	::new(pMemory) T(src);
}

template <class T>
inline void Destruct(T* pMemory)
{
	pMemory->~T();
}
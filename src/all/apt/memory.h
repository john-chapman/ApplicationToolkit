#pragma once
#ifndef apt_memory_h
#define apt_memory_h

#include <apt/apt.h>

namespace apt {

// Allocate _size bytes with _align aligment.
void* malloc_aligned(uint _size, uint _align);

// Reallocate memory previously allocated via malloc_aligned (or allocate a new block if _p is null). The alignment of a previously 
// allocated block may not be changed.
void* realloc_aligned(void* _p, uint _size, uint _align);

// Free memory previous allocated via malloc_aligned().
void free_aligned(void* _p);


namespace internal {
	template <uint kAlignment> struct aligned_base;
		template<> struct APT_ALIGN(1)   aligned_base<1>   {};
		template<> struct APT_ALIGN(2)   aligned_base<2>   {};
		template<> struct APT_ALIGN(4)   aligned_base<4>   {};
		template<> struct APT_ALIGN(8)   aligned_base<8>   {};
		template<> struct APT_ALIGN(16)  aligned_base<16>  {};
		template<> struct APT_ALIGN(32)  aligned_base<32>  {};
		template<> struct APT_ALIGN(64)  aligned_base<64>  {};
		template<> struct APT_ALIGN(128) aligned_base<128> {};
} // namespace internal

////////////////////////////////////////////////////////////////////////////////
// aligned
// Mixin class, provides template-based memory alignment for deriving classes.
// Use cautiously, especially with multiple inheritance.
// Usage:
//
//    class Foo: public aligned<Foo, 16>
//    { // ...
//
// \note Alignment can only be increased. If the deriving class has a higher
//    natural alignment than kAlignment, the higher alignment is used.
////////////////////////////////////////////////////////////////////////////////
template <typename tType, uint kAlignment>
struct aligned: private internal::aligned_base<kAlignment>
{
	aligned()                                           { APT_ASSERT((uint)this % kAlignment == 0); }

	// \note malloc_aligned is called with APT_ALIGNOF(tType) which will be the min of the natural alignment of tType and kAlignment
	void* operator new(std::size_t _size)               { return malloc_aligned(_size, APT_ALIGNOF(tType)); }
	void  operator delete(void* _ptr)                   { free_aligned(_ptr); }
	void* operator new[](std::size_t _size)             { return malloc_aligned(_size, APT_ALIGNOF(tType)); }
	void  operator delete[](void* _ptr)                 { free_aligned(_ptr); }
	void* operator new(std::size_t _size, void* _ptr)   { APT_ASSERT((uint)_ptr % kAlignment == 0); return _ptr; }
	void  operator delete(void*, void*)                 { ; } // dummy, matches placement new

};

////////////////////////////////////////////////////////////////////////////////
// storage
// Provides aligned storage for kCount objects of type tType. Suitable for 
// allocating static blocks of uninitialized memory for use with placement
// new.
////////////////////////////////////////////////////////////////////////////////
template <typename tType, uint kCount>
class storage: private aligned< storage<tType, kCount>, APT_ALIGNOF(tType) >
{
	char m_buf[sizeof(tType) * kCount];
public:
	storage(): aligned< storage<tType, kCount>, APT_ALIGNOF(tType) >() {}
	operator tType*() { return (tType*)m_buf; }
};

} // namespace apt

#endif // apt_memory_h

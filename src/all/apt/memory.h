#pragma once

#include <apt/apt.h>

#include <cstring>

#define APT_MALLOC(size)                        (apt::internal::malloc(size))
#define APT_REALLOC(ptr, size)                  (apt::internal::realloc(ptr, size))
#define APT_FREE(ptr)                           (apt::internal::free(ptr))
#define APT_MALLOC_ALIGNED(size, align)         (apt::internal::malloc_aligned(size, align))
#define APT_REALLOC_ALIGNED(ptr, size, align)   (apt::internal::realloc_aligned(ptr, size, align))
#define APT_FREE_ALIGNED(ptr)                   (apt::internal::free_aligned(ptr))
#define APT_NEW(type)                           (new type)
#define APT_NEW_ARRAY(type, count)              (new type[count])
#define APT_DELETE(ptr)                         (delete ptr)
#define APT_DELETE_ARRAY(ptr)                   (delete[] ptr)

namespace apt { namespace internal {

void* malloc(size_t _size);
void* realloc(void* _ptr, size_t _size);
void  free(void* _ptr);
void* malloc_aligned(size_t _size, size_t _align);
void* realloc_aligned(void* _ptr, size_t _size, size_t _align);
void  free_aligned(void* _ptr);

template <size_t kAlignment> struct aligned_base;
	template<> struct alignas(1)   aligned_base<1>   {};
	template<> struct alignas(2)   aligned_base<2>   {};
	template<> struct alignas(4)   aligned_base<4>   {};
	template<> struct alignas(8)   aligned_base<8>   {};
	template<> struct alignas(16)  aligned_base<16>  {};
	template<> struct alignas(32)  aligned_base<32>  {};
	template<> struct alignas(64)  aligned_base<64>  {};
	template<> struct alignas(128) aligned_base<128> {};

} } // namespace apt::internal

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// aligned
// Mixin class, provides template-based memory alignment for deriving classes.
// Use cautiously, especially with multiple inheritance.
// Usage:
//
//    class Foo: public aligned<Foo, 16>
//    { // ...
//
// Alignment can only be increased. If the deriving class has a higher natural 
// alignment than kAlignment, the higher alignment is used.
////////////////////////////////////////////////////////////////////////////////
template <typename tType, size_t kAlignment>
struct aligned: private internal::aligned_base<kAlignment>
{
	aligned()                                           { APT_STRICT_ASSERT((size_t)this % kAlignment == 0); }

	// malloc_aligned is called with alignof(tType) which will be the min of the natural alignment of tType and kAlignment
	void* operator new(size_t _size)                    { return APT_MALLOC_ALIGNED(_size, alignof(tType)); }
	void  operator delete(void* _ptr)                   { APT_FREE_ALIGNED(_ptr); }
	void* operator new[](size_t _size)                  { return APT_MALLOC_ALIGNED(_size, alignof(tType)); }
	void  operator delete[](void* _ptr)                 { APT_FREE_ALIGNED(_ptr); }
	void* operator new(size_t _size, void* _ptr)        { APT_STRICT_ASSERT((size_t)_ptr % kAlignment == 0); return _ptr; }
	void  operator delete(void*, void*)                 { ; } // dummy, matches placement new

};

////////////////////////////////////////////////////////////////////////////////
// storage
// Provides aligned storage for kCount objects of type tType. Suitable for 
// allocating static blocks of uninitialized memory for use with placement
// new.
////////////////////////////////////////////////////////////////////////////////
template <typename tType, size_t kCount>
class storage: private aligned< storage<tType, kCount>, alignof(tType) >
{
	char m_buf[sizeof(tType) * kCount];
public:
	storage(): aligned< storage<tType, kCount>, alignof(tType) >() {}
	operator tType*() { return (tType*)m_buf; }
};

} // namespace apt

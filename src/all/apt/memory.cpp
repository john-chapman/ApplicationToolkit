#include <apt/memory.h>

#include <cstdlib>

#if 1
	void* operator new(size_t _size)
	{ 
		return APT_MALLOC(_size); 
	}
	void  operator delete(void* _ptr)
	{ 
		APT_FREE(_ptr); 
	}
	void* operator new[](size_t _size)
	{ 
		return APT_MALLOC(_size); 
	}
	void  operator delete[](void* _ptr)
	{
		APT_FREE(_ptr); 
	}
#endif

void* apt::internal::malloc(size_t _size)
{
	return ::malloc(_size);
}

void* apt::internal::realloc(void* _ptr, size_t _size)
{
	return ::realloc(_ptr, _size);
}

void apt::internal::free(void* _ptr)
{
	::free(_ptr);
}

void* apt::internal::malloc_aligned(size_t _size, size_t _align) 
{
#ifdef APT_COMPILER_MSVC
	return _aligned_malloc(_size, _align);
#else
	aligned_alloc(_align, _size);
#endif
}

void* apt::internal::realloc_aligned(void* _ptr, size_t _size, size_t _align)
{
#ifdef APT_COMPILER_MSVC
	return _aligned_realloc(_ptr, _size, _align);
#else
	if (_p) {
		return realloc(_p, _size);
	} else {
		return malloc_aligned(_size, _align);
	}
#endif
}

void apt::internal::free_aligned(void* _ptr) 
{
#ifdef APT_COMPILER_MSVC
	_aligned_free(_ptr);
#else
	free(_ptr);
#endif
}

// EASTL new[] overloads
#include <EABase/eabase.h>
#include <stddef.h>
#include <new>
#if defined(EA_COMPILER_NO_EXCEPTIONS) && (!defined(__MWERKS__) || defined(_MSL_NO_THROW_SPECS)) && !defined(EA_COMPILER_RVCT)
	#define THROW_SPEC_0    // Throw 0 arguments
	#define THROW_SPEC_1(x) // Throw 1 argument
#else
	#define THROW_SPEC_0    throw()
	#define THROW_SPEC_1(x) throw(x)
#endif

void* operator new[](size_t size, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) THROW_SPEC_1(std::bad_alloc)
{
	return APT_MALLOC(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* /*name*/, int flags, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/) THROW_SPEC_1(std::bad_alloc)
{
#ifdef APT_COMPILER_MSVC
	return _aligned_offset_malloc(size, alignment, alignmentOffset);
#else
 // \todo no standard 'offset' version, implement via aligned_alloc
	APT_ASSERT(false);
#endif
}
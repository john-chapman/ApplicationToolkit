#pragma once
#ifndef apt_h
#define apt_h

#define APT_VERSION "0.09"

// Compiler
#if defined(__GNUC__)
	#define APT_COMPILER_GNU
#elif defined(_MSC_VER)
	#define APT_COMPILER_MSVC
#else
	#error apt: Compiler not defined
#endif

// Platform 
#if defined(_WIN32) || defined(_WIN64)
	#define APT_PLATFORM_WIN
#else
	#error apt: Platform not defined
#endif

// Architecture
#if defined(_M_X64) || defined(__x86_64)
	#define APT_DCACHE_LINE_SIZE 64
#else
	#error apt: Architecture not defined
#endif



#if defined(APT_COMPILER_GNU)
	#define APT_ALIGN(a) __attribute__((aligned(a)))
#elif defined(APT_COMPILER_MSVC)
	#define APT_ALIGN(a) __declspec(align(a))
#else
	#define APT_ALIGN(a) alignas(a)
#endif

#if defined(APT_COMPILER_GNU)
	#define APT_ALIGNOF(t) __alignof__(t)
#elif defined(APT_COMPILER_MSVC)
	#define APT_ALIGNOF(t) __alignof(t)
#else
	#define APT_ALIGNOF(t) alignof(t)
#endif

#if defined(APT_COMPILER_GNU)
	#define APT_THREAD_LOCAL __thread
#elif defined(APT_COMPILER_MSVC)
	#define APT_THREAD_LOCAL __declspec(thread)
#else
	#define APT_THREAD_LOCAL thread_local
#endif

#if defined(APT_COMPILER_GNU)
	#define if_likely(e)   if ( __builtin_expect(!!(e), 1) )
	#define if_unlikely(e) if ( __builtin_expect(!!(e), 0) )
//#elif defined(APT_COMPILER_MSVC)
  // not defined for MSVC
#else
	#define if_likely(e)   if(!!(e))
	#define if_unlikely(e) if(!!(e))
#endif

#ifndef __COUNTER__
// __COUNTER__ is non-standard, so replace with __LINE__ if it's unavailable
	#define __COUNTER__ __LINE__
#endif
#define APT_TOKEN_CONCATENATE_(_t0, _t1) _t0 ## _t1
#define APT_TOKEN_CONCATENATE(_t0, _t1)  APT_TOKEN_CONCATENATE_(_t0, _t1)
#define APT_UNIQUE_NAME(_base) APT_TOKEN_CONCATENATE(_base, __COUNTER__)

#define APT_ARRAY_COUNT(_arr) apt::internal::ArrayCount(_arr)

// Execute a code block once, e.g. APT_ONCE { foo; bar; }
// \note This is not thread safe.
#define APT_ONCE  for (static bool _done = false; _done ? false : _done = true; )

namespace apt {

enum AssertBehavior
{
	AssertBehavior_Break, 
	AssertBehavior_Continue
};

// Typedef for assert callbacks. See DefaultAssertCallback for a description of the function arguments.
typedef AssertBehavior (AssertCallback)(const char* _e, const char* _msg, const char* _file, int _line);

// Return the current assert callback. Default is DefaultAssertCallback.
AssertCallback* GetAssertCallback();

// Set the function to be called when asserts fail, which may be 0.
void SetAssertCallback(AssertCallback* _callback);

// Default assert callback, print message via APT_LOG_ERR(). Always returns AssertBehavior_Break.
AssertBehavior DefaultAssertCallback(const char* _e, const char* _msg,  const char* _file,  int _line);

} // namespace apt


namespace apt { namespace internal {

AssertBehavior AssertAndCallback(const char* _e, const char* _file, int _line, const char* _msg, ...);
const char* StripPath(const char* _path);

template <typename tType, unsigned kCount>
inline constexpr unsigned ArrayCount(const tType (&)[kCount]) { return kCount; }

} } // namespace apt::internal

#define APT_UNUSED(x) do { (void)sizeof(x); } while (0)
#ifdef APT_DEBUG
	#ifdef APT_COMPILER_MSVC
		#define APT_BREAK() __debugbreak()
	#else
		#include <cstdlib>
		#define APT_BREAK() abort()
	#endif

	#define APT_ASSERT_MSG(e, msg, ...) \
		do { \
			if_unlikely (!(e)) { \
				if (apt::internal::AssertAndCallback(#e, __FILE__, __LINE__, msg, ## __VA_ARGS__) == apt::AssertBehavior_Break) \
				{ APT_BREAK(); } \
			} \
		} while(0)

	#define APT_ASSERT(e)                 APT_ASSERT_MSG(e, 0, 0)
	#define APT_VERIFY_MSG(e, msg, ...)   APT_ASSERT_MSG(e, msg, ## __VA_ARGS__)
	#define APT_VERIFY(e)                 APT_VERIFY_MSG(e, 0, 0)

#else
	#define APT_ASSERT_MSG(e, msg, ...)   do { APT_UNUSED(e); APT_UNUSED(msg); } while(0)
	#define APT_ASSERT(e)                 do { APT_UNUSED(e); } while(0)
	#define APT_VERIFY_MSG(e, msg, ...)   do { (void)(e); APT_UNUSED(msg); } while(0)
	#define APT_VERIFY(e)                 (void)(e)

#endif // APT_DEBUG

#define APT_STATIC_ASSERT(e) { (void)sizeof(char[(e) ? 1 : -1]); }

#include <apt/types.h>

namespace apt {

// Forward declarations
class ArgList;
template <typename tType> class Factory;
class File;
class FileSystem;
class Image;
class Ini;
class Json;
class MemoryPool;
template <typename tType> class PersistentVector;
template <typename tType> class Pool;
template <typename PRNG>  class Rand;
template <typename tType> class RingBuffer;
class Serializer;
	class SerializerJson;
class StringBase;
	template <uint kCapacity> class String;
class StringHash;
class TextParser;
class Timestamp;
class DateTime;

////////////////////////////////////////////////////////////////////////////////
// non_copyable
// Mixin class, forces a derived class to be non-copyable.
// \note The template parameter permits Empty Base Optimization (see
//   http://en.wikibooks.org/wiki/More_C++_Idioms/Non-copyable_Mixin).
////////////////////////////////////////////////////////////////////////////////
template <typename tType>
class non_copyable
{
protected:
	inline  non_copyable() {}
	inline ~non_copyable() {}
private:
	non_copyable(const non_copyable&);
	non_copyable& operator=(const non_copyable&);
};


////////////////////////////////////////////////////////////////////////////////
// non_instantiable
// Mixin class, forces a derived class to be non-instantiable. Note that by
// definition a non-instantiable class is also non-copyable.
// \note The template parameter permits Empty Base Optimization (see
//   http://en.wikibooks.org/wiki/More_C++_Idioms/Non-copyable_Mixin).
////////////////////////////////////////////////////////////////////////////////
template <typename tType>
class non_instantiable: private non_copyable<tType>
{
private:
	non_instantiable();
};

} // namespace apt

#endif // apt_h

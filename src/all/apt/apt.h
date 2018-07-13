#pragma once

#define APT_VERSION "0.15"

#include <apt/config.h>

// \deprecated - prefer to use the C++11 versions directly
#define APT_ALIGN(x)     alignas(x)
#define APT_ALIGNOF(x)   alignof(x)
#define APT_THREAD_LOCAL thread_local

#if APT_COMPILER_GNU
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
// This is not thread safe.
#define APT_ONCE  for (static bool _done = false; _done ? false : _done = true; )

namespace apt {

enum AssertBehavior
{
	AssertBehavior_Break, 
	AssertBehavior_Continue
};

// Typedef for assert callbacks.
typedef AssertBehavior (AssertCallback)(const char* _expr, const char* _msg, const char* _file, int _line);

// Return the current assert callback. Default is DefaultAssertCallback.
AssertCallback* GetAssertCallback();

// Set the function to be called when asserts fail, which may be 0.
void SetAssertCallback(AssertCallback* _callback);

// Default assert callback, print message via APT_LOG_ERR(). Always returns AssertBehavior_Break.
AssertBehavior DefaultAssertCallback(const char* _expr, const char* _msg,  const char* _file,  int _line);

} // namespace apt


namespace apt { namespace internal {

AssertBehavior AssertAndCallback(const char* _expr, const char* _file, int _line, const char* _msg, ...);
const char* StripPath(const char* _path);

template <typename tType, unsigned kCount>
inline constexpr unsigned ArrayCount(const tType (&)[kCount]) { return kCount; }

} } // namespace apt::internal

#define APT_UNUSED(x) do { (void)sizeof(x); } while(0)
#if APT_ENABLE_ASSERT
	#if APT_COMPILER_MSVC
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

#endif // APT_ENABLE_ASSERT

#if APT_ENABLE_STRICT_ASSERT
	#define APT_STRICT_ASSERT(e) APT_ASSERT(e)
#else
	#define APT_STRICT_ASSERT(e) do { APT_UNUSED(e); } while(0)
#endif

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

typedef String<128> PathStr;

////////////////////////////////////////////////////////////////////////////////
// non_copyable
// Mixin class, forces a derived class to be non-copyable.
// The template parameter permits Empty Base Optimization (see
//   http://en.wikibooks.org/wiki/More_C++_Idioms/Non-copyable_Mixin).
////////////////////////////////////////////////////////////////////////////////
template <typename tType>
class non_copyable
{
protected:
	non_copyable() = default;
	~non_copyable() = default;

	non_copyable(const non_copyable&) = delete;
	non_copyable& operator=(const non_copyable&) = delete;
};

} // namespace apt

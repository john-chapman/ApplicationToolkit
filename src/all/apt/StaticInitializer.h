#pragma once

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// StaticInitializer
// Implementation of the Nifty/Schwarz counter idiom (see
//   https://john-chapman.github.io/2016/09/01/static-initialization.html). 
// Usage:
//
// // in the .h
//    class Foo
//    {
//       APT_DECALRE_STATIC_INIT_FRIEND(Foo); // give StaticInitializer access to private functions
//       void Init();
//       void Shutdown();
//    };
//    APT_DECLARE_STATIC_INIT(Foo);
//
// // in the .cpp
//    APT_DEFINE_STATIC_INIT(Foo, Foo::Init, Foo::Shutdown);
//
// Note that the use of APT_DECLARE_STATIC_INIT_FRIEND() is optional if the init
// and shutdown methods aren't private members.
//
// Init() should not construct any non-trivial static objects as the order of 
// initialization relative to StaticInitializer cannot be guaranteed. This 
// means that static objects initialized during Init() may subsequently be 
// default-initialized, overwriting the value set by Init(). To get around this, 
// use heap allocation or the storage class (memory.h):
//
//   #inclide <apt/memory.h>
//   static storage<Bar, 1> s_bar;
//
//   void Foo::Init()
//   {
//      new(s_bar) Bar();
//      // ..
////////////////////////////////////////////////////////////////////////////////
template <typename tType>
class StaticInitializer
{
public:
	StaticInitializer()
	{ 
		if_unlikely (++s_initCounter == 1) {
			Init();
		} 
	}
	~StaticInitializer()
	{
		if_unlikely (--s_initCounter == 0) { 
			Shutdown();
		} 
	}
private:
	static int  s_initCounter;

	static void Init();
	static void Shutdown();
};
#define APT_DECLARE_STATIC_INIT_FRIEND(_type) \
	friend class apt::StaticInitializer<_type>
#define APT_DECLARE_STATIC_INIT(_type) \
	static apt::StaticInitializer<_type> _type ## _StaticInitializer
#define APT_DEFINE_STATIC_INIT(_type, _onInit, _onShutdown) \
	template <> int  apt::StaticInitializer<_type>::s_initCounter; \
	template <> void apt::StaticInitializer<_type>::Init()     { _onInit(); } \
	template <> void apt::StaticInitializer<_type>::Shutdown() { _onShutdown(); }

} // namespace apt

#pragma once

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// static_initializer
// Implementation of the Nifty/Schwarz counter idiom (see
//   https://john-chapman.github.io/2016/09/01/static-initialization.html). 
// Usage:
//
// // in the .h
//    class Foo
//    {
//       APT_DECALRE_STATIC_INIT_FRIEND(Foo); // give static_initializer access to private functions
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
// initialization relative to static_initializer cannot be guaranteed. This 
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
class static_initializer
{
public:
	static_initializer()
	{ 
		if_unlikely (++s_initCounter == 1) {
			Init();
		} 
	}
	~static_initializer()
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
#define APT_DECLARE_STATIC_INIT_FRIEND(_type) friend class apt::static_initializer<_type>
#define APT_DECLARE_STATIC_INIT(_type) static apt::static_initializer<_type> _type ## _static_initializer
#define APT_DEFINE_STATIC_INIT(_type, _onInit, _onShutdown) \
	int  apt::static_initializer<_type>::s_initCounter; \
	void apt::static_initializer<_type>::Init() { _onInit(); } \
	void apt::static_initializer<_type>::Shutdown() { _onShutdown(); }

} // namespace apt

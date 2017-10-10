#pragma once
#ifndef apt_time_h
#define apt_time_h

#include <apt/apt.h>
#include <apt/static_initializer.h>


namespace apt {

////////////////////////////////////////////////////////////////////////////////
// Time
// Global time functions.
////////////////////////////////////////////////////////////////////////////////
class Time
{
public:
	// High-resolution time stamp. Use for interval measurements.
	static Timestamp GetTimestamp();
	
	// High-resolution date-time, synchronized to UTC.
	static DateTime GetDateTime();
	
	// Frequency of the system timer in ticks/second.
	static sint64 GetSystemFrequency();
	
	// Interval since the application began.
	static Timestamp GetApplicationElapsed();

	static void Init();
	static void Shutdown();

};
APT_DECLARE_STATIC_INIT(Time, Time::Init, Time::Shutdown);

////////////////////////////////////////////////////////////////////////////////
// Timestamp
// High-resolution unsynchronized timestamp, with functions for converting 
// between ticks and seconds/milliseconds/microseconds. Use for interval
// measurments.
////////////////////////////////////////////////////////////////////////////////
class Timestamp
{
	friend class Time;
public:
	// Default/value-initializing ctor.
	Timestamp(sint64 _raw = 0): m_raw(_raw) {}

	// Raw time value in system-dependent units.
	sint64 getRaw() const { return m_raw; }

	// Raw value converted to seconds.
	double asSeconds() const;

	// Raw value converted to milliseconds (10^-3s).
	double asMilliseconds() const;

	// Raw value converted to microseconds (10^-6s).
	double asMicroseconds() const;
	
	// Return a string with an appropriate units e.g. "2.43s", "17.2ms", "400us".
	// \note Returns a ptr to a local static buffer - for normal use this should 
	//    be fine, just print the string and don't keep the ptr.
	const char*     asString() const;

	const Timestamp operator- (const Timestamp& rhs) const  { return m_raw -  rhs.m_raw; }
	const Timestamp operator+ (const Timestamp& rhs) const  { return m_raw +  rhs.m_raw; }
	Timestamp&      operator-=(const Timestamp& rhs)        { m_raw -= rhs.m_raw; return *this; }
	Timestamp&      operator+=(const Timestamp& rhs)        { m_raw += rhs.m_raw; return *this; }

	bool            operator> (const Timestamp& rhs) const  { return m_raw >  rhs.m_raw; }
	bool            operator>=(const Timestamp& rhs) const  { return m_raw >= rhs.m_raw; }
	bool            operator< (const Timestamp& rhs) const  { return m_raw <  rhs.m_raw; }
	bool            operator<=(const Timestamp& rhs) const  { return m_raw <= rhs.m_raw; }

private:
	sint64 m_raw;

};


////////////////////////////////////////////////////////////////////////////////
// DateTime
// High-resolution datetime, synchronized to UTC.
////////////////////////////////////////////////////////////////////////////////
class DateTime
{
	friend class Time;
public:
	// Default/value-initializing ctor.
	DateTime(sint64 _raw = 0ll): m_raw(_raw) {}

	// Raw time value in system-dependent units.
	sint64 getRaw() const { return m_raw; }

	sint32 getYear() const;
	sint32 getMonth() const;
	sint32 getDay() const;
	sint32 getHour() const;
	sint32 getMinute() const;
	sint32 getSecond() const;
	sint32 getMillisecond() const;

	// Return a formatted string. The default formatting is ISO 8601, however a format 
	// string may be supplied using the following specifiers:
	//   Specifier | Value
	//   ==========+==================================================
	//    %d       | Zero-padded day of month in [01,31]
	//    %H       | Zero-padded hour in [00,24]
	//    %m       | Zero-padded month in [01,12]
	//    %M       | Zero-padded minute in [00,59]
	//    %s       | Zero-padded millisecond in [0000,9999]
	//    %S       | Zero-padded second in [00,59]
	//    %Y       | Year
	// E.g. ISO 8601 format would be "%Y-%m-%dT%H:%M:%SZ".
	// \note Returns a ptr to a local static buffer - for normal use this should 
	//    be fine, just print the string and don't keep the ptr.
	const char*    asString(const char* _format = nullptr) const;

	const DateTime operator- (const DateTime& rhs) const  { return m_raw -  rhs.m_raw; }
	const DateTime operator+ (const DateTime& rhs) const  { return m_raw +  rhs.m_raw; }
	DateTime&      operator-=(const DateTime& rhs)        { m_raw -= rhs.m_raw; return *this; }
	DateTime&      operator+=(const DateTime& rhs)        { m_raw += rhs.m_raw; return *this; }

	bool           operator> (const DateTime& rhs) const  { return m_raw >  rhs.m_raw; }
	bool           operator>=(const DateTime& rhs) const  { return m_raw >= rhs.m_raw; }
	bool           operator< (const DateTime& rhs) const  { return m_raw <  rhs.m_raw; }
	bool           operator<=(const DateTime& rhs) const  { return m_raw <= rhs.m_raw; }

private:	
	sint64 m_raw;

};

////////////////////////////////////////////////////////////////////////////////
// AutoTimer
// Scoped timer. Measures the time between ctor and dtor, logs the interval in
// the dtor. Use APT_AUTOTIMER_DBG to declare an AutoTimer instance for debug 
// builds only.
//
// Auto timers may be nested in which case the result is only logged when the
// outer timer is destroyed, e.g.:
//   Outer -- 20ms
//     Inner -- 10ms
//     Inner -- 10ms
////////////////////////////////////////////////////////////////////////////////
class AutoTimer
{
	int        m_stackIndex;
	Timestamp  m_start;
public:
	AutoTimer(const char* _fmt, ...);
	~AutoTimer();
};
#define APT_AUTOTIMER(...) apt::AutoTimer APT_UNIQUE_NAME(_aptAutoTimer_)(__VA_ARGS__)
#ifdef APT_DEBUG
	#define APT_AUTOTIMER_DBG(...) APT_AUTOTIMER(__VA_ARGS__)
#else
	#define APT_AUTOTIMER_DBG(...) do { } while (0)
#endif

} // namespace apt

#endif // apt_Time_h

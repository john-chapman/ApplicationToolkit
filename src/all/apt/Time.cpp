#include <apt/Time.h>

#include <apt/String.h>

using namespace apt;

const char* Timestamp::asString() const
{
	static String<64> s_buf;
	s_buf.clear();
	float x = (float)asSeconds();
	if (x >= 1.0f) {
		s_buf.setf("%1.3fs", x);
	} else {
		x = (float)asMilliseconds();
		if (x >= 0.1f) {
			s_buf.setf("%1.2fms", x);
		} else {
			x = (float)asMicroseconds();
			s_buf.setf("%1.0fus", x);
		}
	}
	return (const char*)s_buf;
}

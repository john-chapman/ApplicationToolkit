#include <apt/File.h>

#include <apt/memory.h>

using namespace apt;

// PUBLIC

void File::setData(const char* _data, uint64 _size)
{
	if (m_data) {
		if (_size > m_dataSize || _size == 0) {
			free(m_data);
			m_data = 0;
		}
	}

	if (!m_data && _size > 0) {
		m_data = (char*)APT_MALLOC(_size);
		APT_ASSERT(m_data);
	}
	if (_data) {
		memcpy(m_data, _data, _size);
	}
	m_dataSize = _size;
}

void File::appendData(const char* _data, uint64 _size)
{
	m_data = (char*)realloc(m_data, m_dataSize + _size);
	if (_data) {
		memcpy(m_data + m_dataSize, _data, _size);
	}
	m_dataSize += _size;
}


// PRIVATE

void File::ctorCommon()
{
	m_data = nullptr;
	m_dataSize = 0;
	m_impl = nullptr;
}

void File::dtorCommon()
{
	if (m_data) {
		APT_FREE(m_data);
		m_data = nullptr;
	}
}

#include <apt/File.h>

#include <apt/memory.h>

namespace apt {

// PUBLIC

void File::setData(const char* _data, uint _size)
{
	m_data.resize(_size + 1);
	if (_data)
	{
		m_data.assign(_data, _data + _size);
	}
	m_data.back() = '\0';
}

void File::appendData(const char* _data, uint _size)
{
	uint currentSize = getDataSize();
	if (currentSize + (_size + 1) > m_data.capacity())
	{
		m_data.resize(currentSize + (_size + 1));
	}
	m_data.insert(m_data.data() + currentSize, _data, _data + _size);
	m_data.back() = '\0';	
}

void File::reserveData(uint _capacity)
{
	m_data.reserve(_capacity);
}

} // namespace apt

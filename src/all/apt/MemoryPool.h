#pragma once
#ifndef apt_MemoryPool_h
#define apt_MemoryPool_h

#include <apt/apt.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// MemoryPool
// See Pool.h for a more user-friendly, templated version of this.
// Provides (de)allocations of objects with O(1) complexity given a fixed 
// object size/alignment.
// Usage:	
//
//    MemoryPool mp(sizeof(Foo), APT_ALIGNOF(Foo), 128);
//    Foo* f = new(mp.alloc()) Foo; // placement new
//    f->~Foo(); // explicit dtor call
//    mp.free(f);
//
// \note Any allocated objects should be released via free() before the
//    MemoryPool is destroyed.
////////////////////////////////////////////////////////////////////////////////
class MemoryPool: private non_copyable<MemoryPool>
{
public:
	// _objectSize must be at least sizeof(void*). _blockSize is the number of new unused objects 
	// to allocate when allocate() cannot service a new request.
	MemoryPool(uint _objectSize, uint _objectAlignment, uint _blockSize);

	// Free all allocated memory. Any allocated objects should be released via free() before the MemoryPool is destroyed.
	~MemoryPool();

	void* alloc();
	void  free(void* _object);

	// Return true if _ptr was allocated from the pool.
	bool isFromPool(const void* _ptr) const;

	// Return true if # used objects is consistent with # accessible free objects.
	bool validate() const;

	friend void swap(MemoryPool& _a, MemoryPool& _b);
	
private:
	uint   m_objectSize, m_objectAlignment, m_blockSize;
	void*  m_nextFree;
	uint   m_usedCount;
	void** m_blocks;
	uint   m_blockCount;

	void allocBlock();

};

} // namespace apt

#endif // apt_MemoryPool_h

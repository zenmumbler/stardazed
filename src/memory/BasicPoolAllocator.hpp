// ------------------------------------------------------------------
// memory::BasicPoolAllocator - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#ifndef SD_MEM_BASICPOOLALLOCATOR_H
#define SD_MEM_BASICPOOLALLOCATOR_H

#include "memory/STLAllocAdapter.hpp"
#include <vector>
#include <memory>

namespace stardazed {


class BasicMemoryPool {
	const size_t blockSize_;
	mutable std::vector<std::unique_ptr<uint8_t[]>> blocks_;
	mutable uint8_t *arena_, *pos_;
	
	static constexpr size_t DefaultBlockSize = 48 * 1024;
	
	void addBlock(size_t sizeInBytes) const {
		blocks_.emplace_back(new uint8_t[sizeInBytes]);
		pos_ = arena_ = blocks_.back().get();
	}
	
	inline void addBlock() const { addBlock(blockSize_); }
	
public:
	BasicMemoryPool(const size_t blockSize)
	: blockSize_ {blockSize}
	{
		addBlock();
	}
	BasicMemoryPool() : BasicMemoryPool(DefaultBlockSize) {}
	
	void* allocate(size_t n) const {
		if (pos_ + n - arena_ > static_cast<ptrdiff_t>(blockSize_)) {
			if (n > blockSize_)
				addBlock(n); // single-use large block
			else
				addBlock();
		}
		
		auto result = pos_;
		pos_ += n;
		return result;
	}
	
	void deallocate(void* p, size_t n) const {
	}
};


// -- STL Adapter
template <typename T>
using BasicPoolAllocator = STLAllocAdapter<T, BasicMemoryPool>;


} // ns stardazed

#endif
#pragma once

#include <map>
#include <functional>

#include "memory_chunk.h"
#include "physical_memory.h"

namespace reven {
namespace vmghost {

class MemoryVirtualBox : public physical_memory {
	typedef std::map<std::uint64_t, MemoryChunk, std::greater<std::uint64_t>> MemoryChunksContainer;

public:
	typedef MemoryChunksContainer::iterator iterator;
	typedef MemoryChunksContainer::const_iterator const_iterator;
	typedef MemoryChunksContainer::value_type value_type;

	MemoryVirtualBox() = default;

	void clear();

	std::size_t chunks_count() const;

	iterator insert(const MemoryChunk& chunk);

	void visit_chunks(std::function<void(const MemoryChunk&)> visitor) const;

private:
	bool do_read(std::uint64_t physical_address, std::uint8_t& data) const final;
	void do_read_buffer(std::uint64_t physical_address, void* buffer, std::size_t size) const final;

	const_iterator findChunk(std::uint64_t physical_address) const;

	MemoryChunksContainer chunks_;

}; // class MemoryVirtualBox

inline void MemoryVirtualBox::clear()
{
	chunks_.clear();
}

inline std::size_t MemoryVirtualBox::chunks_count() const
{
	return chunks_.size();
}

inline MemoryVirtualBox::const_iterator MemoryVirtualBox::findChunk(std::uint64_t physical_address) const
{
	auto where = chunks_.lower_bound(physical_address);

	if (where != chunks_.end()
	    and (physical_address > where->second.physical_address()
	         + where->second.size_in_memory())) {
		return chunks_.end();
	}

	return where;
}
}
} // namespace reven::vmghost

#include <memory_virtualbox.h>

#include <cstring>
#include <cassert>

namespace reven {
namespace vmghost {

/**
 * @details It implements the trick proposed by Scott Meyers for efficient
 *      insert or update. The problem with a map is that the subscript
 *      operator is *not* efficient when inserting. This implementation
 *      compensates this problem.
 *
 * @see Effective STL - Item 24
 */
MemoryVirtualBox::iterator MemoryVirtualBox::insert(const MemoryChunk& chunk)
{
	iterator lowerBound = chunks_.lower_bound(chunk.physical_address());

	if ((lowerBound != chunks_.end()) && !(chunks_.key_comp()(chunk.physical_address(),
	                                                          lowerBound->first))) {
		lowerBound->second = std::move(chunk);
		return lowerBound;
	} else {
		return chunks_.insert(lowerBound, value_type(chunk.physical_address(), chunk));
	}
}

bool MemoryVirtualBox::do_read(std::uint64_t physical_address, std::uint8_t& output) const
{
	do_read_buffer(physical_address, &output, 1);

	//! @todo May be bold, but considering that it never fails.
	return true;
}

void MemoryVirtualBox::do_read_buffer(std::uint64_t physical_address, void* buffer, std::size_t size) const
{
	auto found_chunk = findChunk(physical_address);
	if (found_chunk == chunks_.end()) {
		std::memset(buffer, 0, size);
		return;
	}

	MemoryChunk const& chunk = found_chunk->second;

	if (not chunk.contains(physical_address)) {
		std::memset(buffer, 0, size);
		return;
	}

	if (not chunk.contains(physical_address + size - 1)) {
		std::memset(buffer, 0, size);
		return;
	}

	chunk.read(physical_address, buffer, size);
}

void MemoryVirtualBox::visit_chunks(std::function<void(const MemoryChunk&)> visitor) const
{
	for (const auto& chunk: chunks_)
		visitor(chunk.second);
}
}
} // reven::vmghost

#include <memory_chunk.h>

using reven::vmghost::MemoryChunk;

void MemoryChunk::read(std::uint64_t physical_address, void* data, std::uint64_t size) const {
	const std::uint64_t upper_physical_address = physical_address + size;
	if (!contains(physical_address) || upper_physical_address - physical_address_ > size_in_memory_) {
		throw std::out_of_range("Trying to read a chunk of memory outside of its range");
	}

	if (physical_address - physical_address_ > size_in_file_) {
		// We are reading unitialized data, so we don't need to change data
		return;
	}

	if (upper_physical_address - physical_address_ > size_in_file_) {
		// A part of the memory we want to read is unitialized, so just shrunk the size to read less
		size = size_in_file_ - (physical_address - physical_address_);
	}

	file_->seekg(offset_in_file_ + (physical_address - physical_address_));
	file_->read(static_cast<char*>(data), size);
}

bool MemoryChunk::contains(std::uint64_t physical_address) const
{
	return (physical_address >= physical_address_) && (physical_address - physical_address_ < size_in_memory_);
}

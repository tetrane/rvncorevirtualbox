#pragma once

#include <fstream>
#include <memory>

namespace reven {
namespace vmghost {

class MemoryChunk {
public:
	MemoryChunk(std::shared_ptr<std::ifstream> file, std::uint64_t offset_in_file, std::uint64_t size_in_file, std::uint64_t physical_address, std::uint64_t size_in_memory)
		: file_(file), offset_in_file_(offset_in_file), size_in_file_(size_in_file), physical_address_(physical_address), size_in_memory_(size_in_memory) {}
	~MemoryChunk() = default;

	std::uint64_t offset_in_file() const { return offset_in_file_; }
	std::uint64_t size_in_file() const { return size_in_file_; }
	std::uint64_t physical_address() const { return physical_address_; }
	std::uint64_t size_in_memory() const { return size_in_memory_; }

	void read(std::uint64_t physical_address, void* data, std::uint64_t size) const;

	bool contains(std::uint64_t physical_address) const;

private:
	std::shared_ptr<std::ifstream> file_;
	std::uint64_t offset_in_file_{0};
	std::uint64_t size_in_file_{0};
	std::uint64_t physical_address_{0};
	std::uint64_t size_in_memory_{0};

}; // class MemoryChunk

/// @todo Create a subclass EmptyMemoryChunk that derives from MemoryChunk,
///     which creates a fake/empty header for inserting between not-empty ones
///     in order to smoothly check for empty memory chunks.

// class EmptyMemoryChunk: public MemoryChunk
// {

// protected:

//   virtual void doOpen(std::string const& /*filepath*/) {}

// }; // class EmptyMemoryChunk

} // namespace vmghost
} // namespace reven

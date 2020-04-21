//!
//! @file physical_memory.h
//! @brief Declares `reven::vmghost::physical_memory`.
//!

#pragma once

#include <cstdint>

namespace reven {
namespace vmghost {

//!
//! @addtogroup vmghost
//! @{
//!

//!
//! Basic interface for service aimed at reading physical data.
//!
class physical_memory {
public:
	typedef std::uint64_t AddressType;

	virtual ~physical_memory() {}

	bool read(std::uint64_t physical_address, std::uint8_t& data) const;

	template <typename ReadTypeSize, typename DataType> bool read(AddressType const& physical_address, DataType& data) const;

	void read_buffer(std::uint64_t physical_address, void* buffer, std::size_t size) const;

	template <typename Media> void serialize(Media& to) const;

	template <typename Media> void deserialize(Media& from);

protected:
	virtual bool do_read(std::uint64_t physical_address, std::uint8_t& data) const = 0;
	virtual void do_read_buffer(std::uint64_t physical_address, void* buffer, std::size_t size) const = 0;

}; // class physical_memory

//!
//! @}
//!

inline void physical_memory::read_buffer(std::uint64_t physical_address, void* buffer, std::size_t size) const
{
	do_read_buffer(physical_address, buffer, size);
}

template <typename Media> inline void physical_memory::serialize(Media& to __attribute__((unused))) const
{
	// nothing to do
}

template <typename Media> inline void physical_memory::deserialize(Media& from __attribute__((unused)))
{
	// nothing to do
}
}
} // namespace reven::vmghost

#include <physical_memory.h>
#include "physical_memory_impl.h"

namespace reven {
namespace vmghost {

bool physical_memory::read(std::uint64_t physical_address, std::uint8_t& data) const
{
	return do_read(physical_address, data);
}

template bool physical_memory::read<std::uint16_t>(AddressType const&, std::uint16_t&) const;
template bool physical_memory::read<std::uint32_t>(AddressType const&, std::uint32_t&) const;
template bool physical_memory::read<std::uint64_t>(AddressType const&, std::uint64_t&) const;

template bool physical_memory::read<std::uint16_t>(AddressType const&, std::uint32_t&) const;
template bool physical_memory::read<std::uint16_t>(AddressType const&, std::uint64_t&) const;

template bool physical_memory::read<std::uint32_t>(AddressType const&, std::uint64_t&) const;
}
} // namespace reven::vmghost

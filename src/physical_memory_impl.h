#pragma once

namespace reven {
namespace vmghost {

template <typename ReadTypeSize, typename DataType>
bool physical_memory::read(AddressType const& physical_address, DataType& data) const
{
	static_assert(sizeof(data) >= sizeof(ReadTypeSize), "Data does not fit the requested size!");

	std::size_t size = sizeof(ReadTypeSize);

	for (AddressType i = 0; i < size; ++i) {
		AddressType shift = sizeof(std::uint8_t) * i;

		std::uint8_t value;

		bool result = read(physical_address + shift, value);

		if (false == result) {
			return result;
		}

		reinterpret_cast<std::uint8_t*>(&data)[shift] = value;
	}

	return true;
}
}
} // namespace reven::vmghost

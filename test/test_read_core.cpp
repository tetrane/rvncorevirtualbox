#include <core_virtualbox.h>

#include <iostream>

#define BOOST_TEST_MODULE core_virtualbox
#include <boost/test/unit_test.hpp>

struct int80_core : public reven::vmghost::core_virtualbox {
	int80_core()
	{
		static const std::string core_path = REVEN_TEST_DATA "/scenarios/int80_nop_int80_nop_x86/int80_nop_int80_nop_x86.core";

		parse(core_path);
	}
};

BOOST_AUTO_TEST_CASE(ReadCore)
{
	struct Test {
		std::uint64_t const address_;
		std::uint64_t const reference_;
	};

	Test tests_suite[] = { { 0x0, 0xf000ff53f000ff53 } };

	for (auto test : tests_suite) {
		std::uint64_t address(test.address_);
		std::uint64_t reference(test.reference_);

		int80_core vm;

		auto memory(vm.physical_memory());

		for (auto cpu = vm.cpu_begin(); cpu != vm.cpu_end(); ++cpu) {
			std::uint64_t value;

			memory->read<std::uint64_t>(address, value);

			BOOST_CHECK(value == reference);

			if (value != reference) {
				std::cout << "Reading memory at physical address " << std::showbase << std::hex << address << std::dec
				          << " does not yield the same value: value is " << std::showbase << std::hex << value << std::dec
				          << " vs. expected value is " << std::showbase << std::hex << address << std::dec << std::endl;
			}
		}
	}
}

BOOST_AUTO_TEST_CASE(ReadNonExistingCore)
{
	reven::vmghost::core_virtualbox core;

	BOOST_CHECK_THROW(core.parse("foo.core2"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(serialization)
{
	int80_core vm;

	std::uint64_t valueBefore;

	{
		auto memory(vm.physical_memory());

		memory->read<std::uint64_t>(0x0, valueBefore);
	}

	{
		std::stringstream stream;

		vm.serialize(stream);
		vm.deserialize(stream);
	}


	std::uint64_t valueAfter;

	{
		auto memory(vm.physical_memory());

		memory->read<std::uint64_t>(0x0, valueAfter);
	}

	BOOST_CHECK(valueBefore == valueAfter);
}

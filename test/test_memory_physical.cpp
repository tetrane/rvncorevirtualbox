#include <physical_memory.h>

#include <iostream>

#define BOOST_TEST_MODULE MemoryPhysical
#include <boost/test/unit_test.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

class Memory : public reven::vmghost::physical_memory {
public:
	Memory(std::string const& filepath) : file_(filepath) {}

	std::size_t size() const { return file_.size(); }

private:
	Memory* doClone() const { return new Memory(*this); }

	bool do_read(std::uint64_t physical_address, std::uint8_t& data) const final
	{
		if (physical_address >= size()) {
			return false;
		}

		::memcpy(&data, file_.data() + physical_address, sizeof(std::uint8_t));

		return true;
	}

	void do_read_buffer(std::uint64_t, void*, std::size_t) const final { throw "not implemented"; }

	boost::iostreams::mapped_file_source file_;

}; // struct Memory

struct MemoryFixture {
	// 1024 bytes
	MemoryFixture() : memory_(REVEN_TEST_DATA "/binaries/testMemoryPhysical.bin") {}

	void checkRead(std::uint64_t offset, std::uint8_t reference)
	{
		std::uint8_t data;

		bool result = memory_.read(offset, data);

		std::cout << std::hex << std::showbase << "Checking read @" << offset
		          << ": reference: " << static_cast<std::uint32_t>(reference)
		          << " vs. data: " << static_cast<std::uint32_t>(data) << std::endl;

		BOOST_CHECK(result);
		BOOST_CHECK_EQUAL(data, reference);
	}

	template <typename ReadSizeType, typename DataType = ReadSizeType>
	void checkRead(std::uint64_t offset, ReadSizeType reference)
	{
		DataType data = 0;

		bool result = memory_.read<ReadSizeType>(offset, data);

		std::cout << std::hex << std::showbase << "Checking read @" << offset << ": reference: " << reference
		          << " vs. data: " << data << std::endl;

		BOOST_CHECK(result);
		BOOST_CHECK_EQUAL(data, reference);
	}

	Memory memory_;
};

BOOST_FIXTURE_TEST_CASE(readAtomicValue, MemoryFixture)
{
	struct TestCase {
		std::uint64_t offset_;
		std::uint8_t reference_;
	};

	TestCase const testsSuite[] = { { 0x0, 0xac }, { 0x1, 0xd0 }, { 0x2, 0x42 }, { 0x3e7, 0x12 }, { 0x3ff, 0x56 } };

	for (auto const test : testsSuite) {
		checkRead(test.offset_, test.reference_);
	}

	std::uint8_t dummy;
	BOOST_CHECK_EQUAL(memory_.read(memory_.size(), dummy), false);
}

BOOST_FIXTURE_TEST_CASE(readArbitraryValueExactSize, MemoryFixture)
{
	struct TestCase {
		std::uint64_t offset_;
		std::tuple<std::uint16_t, std::uint32_t, std::uint64_t> references_;
	};

	TestCase const testsSuite[] = { { 0x0, std::make_tuple(0xd0ac, 0xa842d0ac, 0x6f09ecf2a842d0ac) },
		                            { 0x1, std::make_tuple(0x42d0, 0xf2a842d0, 0x706f09ecf2a842d0) },
		                            { 0x3f8, std::make_tuple(0x5fca, 0x8a0d5fca, 0x56cb48c88a0d5fca) } };

	for (auto const test : testsSuite) {
		checkRead<std::uint16_t>(test.offset_, std::get<0>(test.references_));
		checkRead<std::uint32_t>(test.offset_, std::get<1>(test.references_));
		checkRead<std::uint64_t>(test.offset_, std::get<2>(test.references_));
	}
}

BOOST_FIXTURE_TEST_CASE(readArbitraryValueBiggerSize, MemoryFixture)
{
	struct TestCase {
		std::uint64_t offset_;
		std::tuple<std::uint16_t, std::uint32_t, std::uint64_t> references_;
	};

	TestCase const testsSuite[] = { { 0x0, std::make_tuple(0xd0ac, 0xa842d0ac, 0x6f09ecf2a842d0ac) },
		                            { 0x1, std::make_tuple(0x42d0, 0xf2a842d0, 0x706f09ecf2a842d0) },
		                            { 0x3f8, std::make_tuple(0x5fca, 0x8a0d5fca, 0x56cb48c88a0d5fca) } };

	for (auto const test : testsSuite) {
		checkRead<std::uint16_t, std::uint64_t>(test.offset_, std::get<0>(test.references_));
		checkRead<std::uint32_t, std::uint64_t>(test.offset_, std::get<1>(test.references_));
		checkRead<std::uint64_t, std::uint64_t>(test.offset_, std::get<2>(test.references_));
	}
}

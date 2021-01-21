//!
//! @file core_virtualbox.h
//! @brief Declaration of class @c reven::vmghost::core_virtualbox.
//!

#pragma once

#include <vector>
#include <fstream>
#include <memory>

#include "cpu_virtualbox.h"
#include "memory_virtualbox.h"
#include "core_virtualbox_def.h"

namespace reven {
namespace vmghost {

//!
//! Represent a VirtualBox core object, loadable from a file.
//!
//! @class core_virtualbox core_virtualbox.h <vmghost/core_virtualbox/core_virtualbox.h>
//!
class core_virtualbox {
	typedef std::vector<cpu_virtualbox> cpu_vector;

public:
	typedef cpu_vector::const_iterator cpu_iterator;

	//! Default constructor.
	core_virtualbox();

	virtual ~core_virtualbox();

	//! The virtual machine physical memory.
	std::shared_ptr<MemoryVirtualBox> physical_memory() const { return memory_; }

	//! The virtual machine cpus.
	cpu_iterator cpu_begin() const { return cpus_.begin(); }
	cpu_iterator cpu_end() const { return cpus_.end(); }

	//! The number of cpu of the core virtual machine.
	std::uint32_t cpu_count() const { return descriptor_.cCpus; }

	//! The core magic value.
	std::uint32_t magic() const { return descriptor_.u32Magic; }

	//! The core format version.
	std::uint32_t format_version() const { return descriptor_.u32FmtVersion; }

	//! The virtualbox version which produce the core.
	std::uint32_t virtualbox_version() const { return descriptor_.u32VBoxVersion; }

	//! The virtualbox revision which produce the core.
	std::uint32_t virtualbox_revision() const { return descriptor_.u32VBoxRevision; }

	//! Writes the core's path.
	template <typename Media> void serialize(Media& to) const;

	//! Reads the core's path.
	template <typename Media> void deserialize(Media& from);

	void parse(std::string const& filepath);

private:
	void read_descriptor(std::uint64_t file_offset);
	void read_cpu(std::uint8_t cpu_nb, std::uint64_t file_offset);
	void read_tetrane_cpu(std::uint8_t cpu_nb, std::uint64_t file_offset);

private:
	//! Path to the core file (used for serialization).
	std::string core_path_;

	//! the core file.
	std::shared_ptr<std::ifstream> file_;

	//! The description of the loaded core.
	vbox::DBGFCOREDESCRIPTOR descriptor_;

	//! Vector of CPU data.
	cpu_vector cpus_;

	//! The memory of the virtual machine.
	std::shared_ptr<MemoryVirtualBox> memory_;

}; // class core_virtualbox

//!
//! @param to Serializer object to save data to.
//!
template <typename Serializer> void core_virtualbox::serialize(Serializer& to) const
{
	// Nothing needs to be saved for memory_ (MemoryVirtualBox): it will parsed again.
	to << core_path_;
}

//!
//! The instance is reset with a new, default, one; then, the core path is extracted from @c from. In turn, the path
//!   is used for extracting remaining data.
//!
//! @param from Deserializer object to extract data from.
//!
//! @see <tt>core_virtualbox::parse()</tt>
//!
template <typename Deserializer> void core_virtualbox::deserialize(Deserializer& from)
{
	// memory_ (MemoryVirtualBox) was not saved; its data is parsed from the core.

	from >> core_path_;

	parse(core_path_);
}
}
} // namespace reven::vmghost

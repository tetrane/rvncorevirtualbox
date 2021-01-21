//!
//! @file core_virtualbox.cpp
//! @brief Runtime definitions for class @c reven::vmghost::core_virtualbox.
//!

#include <core_virtualbox.h>
#include <core_virtualbox_def.h>

#include <elf.h>

#include <iostream>

#define ALIGN_UP(value, alignment) (((value) + (alignment) - 1) & ~((alignment) - 1))

namespace reven {
namespace vmghost {

core_virtualbox::core_virtualbox() : file_(new std::ifstream), memory_(new MemoryVirtualBox)
{
}

core_virtualbox::~core_virtualbox() = default;

void core_virtualbox::read_descriptor(std::uint64_t file_offset) {
	static_assert(std::is_trivially_copyable<vbox::DBGFCOREDESCRIPTOR>::value,
	              "CoreDescription is not trivially copyable.");

	file_->seekg(file_offset);
	file_->read(reinterpret_cast<char*>(&descriptor_), sizeof(descriptor_));

	// Perform basic sanity checking on core descriptor.
	if (descriptor_.u32Magic != vbox::DBGFCORE_MAGIC) {
		throw std::runtime_error("Unsupported core format.");
	}

	if (descriptor_.u32FmtVersion < vbox::DBGFCORE_FMT_VERSION_COMPAT or
	    descriptor_.u32FmtVersion > vbox::DBGFCORE_FMT_VERSION) {
		throw std::runtime_error("Unsupported core version.");
	}

	cpus_.resize(descriptor_.cCpus);
}

void core_virtualbox::read_cpu(std::uint8_t cpu_nb, std::uint64_t file_offset) {
	if (cpu_nb >= cpus_.size()) {
		throw std::runtime_error("More cpu than expected");
	}

	vbox::DBGFCORECPU context;

	file_->seekg(file_offset);
	file_->read(reinterpret_cast<char*>(&context), sizeof(context));

	cpus_[cpu_nb].set_context(context);
}

void core_virtualbox::read_tetrane_cpu(std::uint8_t cpu_nb, std::uint64_t file_offset) {
	if (cpu_nb >= cpus_.size()) {
		throw std::runtime_error("More cpu than expected");
	}

	std::uint64_t magic;
	std::uint64_t size;
	tetrane_cpu_info tetrane_context;

	file_->seekg(file_offset);

	file_->read(reinterpret_cast<char*>(&magic), sizeof(magic));

	if (magic != vbox::TETRANE_SECTION_MAGIC) {
		throw std::runtime_error("Bad magic for tetrane cpu section.");
	}

	file_->read(reinterpret_cast<char*>(&size), sizeof(size));

	// We don't need to check the size for now

	file_->read(reinterpret_cast<char*>(&tetrane_context), sizeof(tetrane_context));

	cpus_[cpu_nb].set_tetrane_context(tetrane_context);
}

// Assuming correct VirtualBox ELF core format.
void core_virtualbox::parse(std::string const& filepath)
{
	file_->close();

	core_path_ = filepath;
	file_->open(filepath);

	if (not file_->is_open()) {
		throw std::runtime_error("Can't open the core file.");
	}

	Elf64_Ehdr ehdr;
	file_->read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));

	std::uint64_t ph_offset = ehdr.e_phoff;

	cpus_.clear();
	memory_->clear();

	std::uint8_t cpu_counter = 0;
	std::uint8_t tetrane_cpu_counter = 0;

	for (std::uint16_t i = 0; i < ehdr.e_phnum; ++i) {
		Elf64_Phdr phdr;

		file_->seekg(ph_offset);
		file_->read(reinterpret_cast<char*>(&phdr), sizeof(phdr));

		if (phdr.p_type == PT_LOAD) {
			memory_->insert(
				MemoryChunk(file_, phdr.p_offset, phdr.p_filesz, phdr.p_paddr, phdr.p_memsz)
			);
		} else if (phdr.p_type == PT_NOTE) {
			std::uint64_t note_offset = 0;

			while (note_offset < phdr.p_filesz) {
				Elf64_Nhdr note;

				file_->seekg(phdr.p_offset + note_offset);
				file_->read(reinterpret_cast<char*>(&note), sizeof(note));

				const std::uint64_t desc_offset = phdr.p_offset + note_offset + sizeof(note) + ALIGN_UP(note.n_namesz, 4);

				if (note.n_type == vbox::NT_VBOXCORE) {
					read_descriptor(desc_offset);
				} else if (note.n_type == vbox::NT_VBOXCPU) {
					read_cpu(cpu_counter, desc_offset);
					++cpu_counter;
				} else if (note.n_type == vbox::TETRANE_CPU_SECTION_NOTE_TYPE) {
					read_tetrane_cpu(tetrane_cpu_counter, desc_offset);
					++tetrane_cpu_counter;
				}

				note_offset += sizeof(note) + ALIGN_UP(note.n_namesz, 4) + ALIGN_UP(note.n_descsz, 4);
			}
		}

		ph_offset += ehdr.e_phentsize;
	}
}
}
} // namespace reven::vmghost

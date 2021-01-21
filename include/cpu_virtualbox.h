#pragma once

#include "core_virtualbox_def.h"

namespace reven {
namespace vmghost {

class cpu_virtualbox {
public:
	cpu_virtualbox() = default;

	explicit cpu_virtualbox(std::uint32_t version, vbox::DBGFCORECPU context)
		: version_(version)
		, context_(context) {}

	cpu_virtualbox(cpu_virtualbox&& rhs) = default;
	~cpu_virtualbox() = default;


	// The version of the passed context should be the same as version(), otherwise the behavior is undefined.
	inline void set_context(const vbox::DBGFCORECPU& context) { context_ = context; }
	inline void set_tetrane_context(const tetrane_cpu_info& tetrane_context) { tetrane_context_ = tetrane_context; }

	inline void set_version(std::uint32_t version) { version_ = version; }
	inline std::uint32_t version() const { return version_; }

	//! @name Paging features
	//! @{

	bool is_paging_enabled() const;
	bool is_pae_enabled() const;
	bool is_pse_enabled() const;
	bool is_smep_enabled() const;
	bool is_pse36_enabled() const;
	bool is_nx_enabled() const;

	//! @}

	std::uint64_t rax() const;
	std::uint64_t rbx() const;
	std::uint64_t rcx() const;
	std::uint64_t rdx() const;

	std::uint64_t rsp() const;
	std::uint64_t rbp() const;
	std::uint64_t rsi() const;
	std::uint64_t rdi() const;

	std::uint64_t r8() const;
	std::uint64_t r9() const;
	std::uint64_t r10() const;
	std::uint64_t r11() const;
	std::uint64_t r12() const;
	std::uint64_t r13() const;
	std::uint64_t r14() const;
	std::uint64_t r15() const;

	//! @name Control registers
	//! @{

	std::uint64_t cr0() const;
	std::uint64_t cr2() const;
	std::uint64_t cr3() const;
	std::uint64_t cr4() const;
	std::uint64_t cr8() const;

	//! @}

	std::uint64_t rip() const;

	//! @name Descriptor tables
	//! @{

	#define GENERATE_DESCRIPTOR_FUNCTIONS(name)	\
		std::uint64_t name##_base() const;		\
		std::uint32_t name##_limit() const;

	//! Global descriptor table.
	GENERATE_DESCRIPTOR_FUNCTIONS(gdtr)
	//! Interrupt descriptor table.
	GENERATE_DESCRIPTOR_FUNCTIONS(idtr)

	#undef GENERATE_DESCRIPTOR_FUNCTIONS

	#define GENERATE_SELECTOR_FUNCTIONS(name)	\
		std::uint16_t name() const;				\
		std::uint64_t name##_base() const;		\
		std::uint32_t name##_limit() const;		\
		std::uint32_t name##_attr() const;		\
		std::uint8_t name##_attr_type() const;	\
		bool name##_attr_desc_type() const;		\
		std::uint8_t name##_attr_dpl() const;	\
		bool name##_attr_present() const;		\
		bool name##_attr_available() const;		\
		bool name##_attr_long() const;			\
		bool name##_attr_def_big() const;		\
		bool name##_attr_granularity() const;

	//! Local descriptor table.
	GENERATE_SELECTOR_FUNCTIONS(ldtr)
	//! TSS descriptor.
	GENERATE_SELECTOR_FUNCTIONS(tr)

	//! @}

	//! @name Segments
	//! @{

	//! Code segment.
	GENERATE_SELECTOR_FUNCTIONS(cs)
	//! Data segment.
	GENERATE_SELECTOR_FUNCTIONS(ds)
	//! Stack segment.
	GENERATE_SELECTOR_FUNCTIONS(ss)
	GENERATE_SELECTOR_FUNCTIONS(gs)
	GENERATE_SELECTOR_FUNCTIONS(fs)
	GENERATE_SELECTOR_FUNCTIONS(es)

	#undef GENERATE_SELECTOR_FUNCTIONS

	//! @}

	//! @name Flags
	//! @{

	std::uint64_t rflags() const;

	//! Carry flag.
	bool carry_flag() const;
	//! Parity flag.
	bool parity_flag() const;
	//! Adjust flag.
	bool adjust_flag() const;
	//! Zero flag.
	bool zero_flag() const;
	//! Sign flag.
	bool sign_flag() const;
	//! Overflow flag.
	bool overflow_flag() const;
	//! Directional flag.
	bool directional_flag() const;
	//! Resume flag.
	bool resume_flag() const;
	//! Trap flag.
	bool trap_flag() const;
	//! Interrupt flag.
	bool interrupt_flag() const;
	//! cpuid flag.
	bool cpuid_flag() const;
	//! iopl flag.
	bool iopl_flag() const;

	//! @}

	bool eflag_reserved_bit1() const;

	//! Ring0 instruction pointer offset that is used by a sysenter instruction (`IA32_SYSENTER_EIP`).
	std::uint64_t sysenter_eip_r0() const;
	//! Ring0 stack pointer that is used by a sysenter instruction (`IA32_SYSENTER_ESP`).
	std::uint64_t sysenter_esp_r0() const;
	//! Ring0 stack segment that is used by a sysenter instruction.
	std::uint64_t sysenter_ss_r0() const;
	//! Ring0 code segment that is used by a sysenter instruction (`IA32_SYSENTER_CS`).
	std::uint64_t sysenter_cs_r0() const;

	//! Code segment.
	std::uint16_t cs_r3() const;
	std::uint16_t ds_r3() const;
	std::uint16_t ss_r3() const;

	//! @name Floating points
	//! @{

	std::uint16_t fpu_status_word() const;
	std::uint16_t fpu_control_word() const;
	std::uint16_t fpu_tag_word() const;
	std::uint8_t fpu_abridged_tags() const;

	bool fpu_c0() const;
	bool fpu_c1() const;
	bool fpu_c2() const;
	bool fpu_c3() const;

	std::uint8_t fpu_top() const;
	std::uint8_t fpu_busy() const;
	std::uint8_t fpu_ie() const;
	std::uint8_t fpu_de() const;
	std::uint8_t fpu_ze() const;
	std::uint8_t fpu_oe() const;
	std::uint8_t fpu_ue() const;
	std::uint8_t fpu_pe() const;
	std::uint8_t fpu_sf() const;
	std::uint8_t fpu_es() const;

	std::uint16_t fpu_fop() const;
	std::uint32_t fpu_ip() const;
	std::uint16_t fpu_cs() const;
	std::uint32_t fpu_dp() const;
	std::uint16_t fpu_ds() const;

	long double fpu_register(std::uint8_t index) const;

	//! @}
	//! @name SIMD (SSE)
	//! @{

	std::uint32_t partial_sse_register(std::uint8_t index, std::uint8_t part_index) const;

	std::uint32_t mxcsr() const;
	std::uint32_t mxcsr_mask() const;

	std::uint8_t mxcsr_ie() const;
	std::uint8_t mxcsr_de() const;
	std::uint8_t mxcsr_ze() const;
	std::uint8_t mxcsr_oe() const;
	std::uint8_t mxcsr_ue() const;
	std::uint8_t mxcsr_pe() const;
	std::uint8_t mxcsr_daz() const;
	std::uint8_t mxcsr_im() const;
	std::uint8_t mxcsr_dm() const;
	std::uint8_t mxcsr_zm() const;
	std::uint8_t mxcsr_om() const;
	std::uint8_t mxcsr_um() const;
	std::uint8_t mxcsr_pm() const;
	std::uint8_t mxcsr_rc() const;
	std::uint8_t mxcsr_fz() const;
	std::uint8_t mxcsr_mm() const;

	//! @}

	//! Retrieves the associated debug register
	std::uint64_t dr(std::uint8_t reg) const;

	std::uint64_t msrEFER() const;
	std::uint64_t msrSTAR() const;
	std::uint64_t msrPAT() const;
	std::uint64_t msrLSTAR() const;
	std::uint64_t msrCSTAR() const;
	std::uint64_t msrSFMASK() const;
	std::uint64_t msrKernelGSBase() const;
	std::uint64_t msrApicBase() const;

	//! Throws: RuntimeError on v5 cores
	std::uint64_t msrTscAux() const;

private:
	std::uint16_t fpu_rebuild_tag_word() const;

	const vbox::X86XSAVEAREA& ext() const;

	std::uint32_t version_;
	vbox::DBGFCORECPU context_;
	tetrane_cpu_info tetrane_context_;

}; // class cpu_virtualbox
}
} // namespace reven::vmghost

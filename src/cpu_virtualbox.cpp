#include <cpu_virtualbox.h>

#include <cstring>

namespace reven {
namespace vmghost {

namespace {

std::uint16_t fpu_get_top(const vbox::X86FXSTATE* p_fpu)
{
	return (p_fpu->FSW >> 11) & 0x7;
}

unsigned int fpu_st_index_for_r(unsigned int i_reg, std::uint16_t top)
{
	return (i_reg - top) & 0x7;
}

std::uint16_t fpu_restore_tag(const vbox::X86FXSTATE* pFpu, unsigned int iReg)
{
	// source: cpumR3RegCalcFpuTagFromFxSave
	// moded to match fpu ri registers to sti in vbox fpu state
	if (!(pFpu->FTW & (1ul << iReg)))
		return 3; /* b11 - empty */

	std::uint16_t top = fpu_get_top(pFpu);
	iReg = fpu_st_index_for_r(iReg, top);

	std::uint16_t const uExp = pFpu->aRegs[iReg].au16[4];
	if (uExp == 0) {
		if (pFpu->aRegs[iReg].au64[0] == 0) /* J & M == 0 */
			return 1;                       /* b01 - zero */
		return 2;                           /* b10 - special */
	}

	if ((uExp & UINT16_C(0x7fff)) == UINT16_C(0x7fff))
		return 2; /* b10 - special */

	if (!(pFpu->aRegs[iReg].au64[0] >> 63)) /* J == 0 */
		return 2;                           /* b10 - special */

	return 0; /* b00 - valid (normal) */
}

} // anonymous-namespace

const vbox::X86XSAVEAREA& cpu_virtualbox::ext() const {
	if (version_ == vbox::DBGFCORE_FMT_VERSIONv6) {
		return context_.v6.ext;
	} else {
		return context_.v5.ext;
	}
}

std::uint16_t cpu_virtualbox::fpu_rebuild_tag_word() const
{
	auto pFpu = &(ext().x87);

	return fpu_restore_tag(pFpu, 0) | (fpu_restore_tag(pFpu, 1) << 2) | (fpu_restore_tag(pFpu, 2) << 4) |
	       (fpu_restore_tag(pFpu, 3) << 6) | (fpu_restore_tag(pFpu, 4) << 8) | (fpu_restore_tag(pFpu, 5) << 10) |
	       (fpu_restore_tag(pFpu, 6) << 12) | (fpu_restore_tag(pFpu, 7) << 14);
}

long double cpu_virtualbox::fpu_register(std::uint8_t r_index) const
{
	long double result = 0.0;
	auto st_index = (r_index + 8 - fpu_top()) & 0x7;
	::memcpy(&result, ext().x87.aRegs[st_index].au8, sizeof(long double));
	return result;
}

std::uint16_t cpu_virtualbox::fpu_status_word() const {
	return ext().x87.FSW;
}
std::uint16_t cpu_virtualbox::fpu_control_word() const
{
	return ext().x87.FCW;
}
std::uint16_t cpu_virtualbox::fpu_tag_word() const
{
	return fpu_rebuild_tag_word();
}
std::uint8_t cpu_virtualbox::fpu_abridged_tags() const
{
	return ext().x87.FTW;
}
std::uint8_t cpu_virtualbox::fpu_ie() const
{
	return (ext().x87.FSW & 1);
}
std::uint8_t cpu_virtualbox::fpu_de() const
{
	return (ext().x87.FSW >> 1) & 1;
}
std::uint8_t cpu_virtualbox::fpu_ze() const
{
	return (ext().x87.FSW >> 2) & 1;
}
std::uint8_t cpu_virtualbox::fpu_oe() const
{
	return (ext().x87.FSW >> 3) & 1;
}
std::uint8_t cpu_virtualbox::fpu_ue() const
{
	return (ext().x87.FSW >> 4) & 1;
}
std::uint8_t cpu_virtualbox::fpu_pe() const
{
	return (ext().x87.FSW >> 5) & 1;
}
std::uint8_t cpu_virtualbox::fpu_sf() const
{
	return (ext().x87.FSW >> 6) & 1;
}
std::uint8_t cpu_virtualbox::fpu_es() const
{
	return (ext().x87.FSW >> 7) & 1;
}
bool cpu_virtualbox::fpu_c0() const
{
	return (ext().x87.FSW >> 8) & 1;
}
bool cpu_virtualbox::fpu_c1() const
{
	return (ext().x87.FSW >> 9) & 1;
}
bool cpu_virtualbox::fpu_c2() const
{
	return (ext().x87.FSW >> 10) & 1;
}
std::uint8_t cpu_virtualbox::fpu_top() const
{
	return (ext().x87.FSW >> 11) & 7;
}
bool cpu_virtualbox::fpu_c3() const
{
	return (ext().x87.FSW >> 14) & 1;
}
std::uint8_t cpu_virtualbox::fpu_busy() const
{
	return (ext().x87.FSW >> 15) & 1;
}

std::uint16_t cpu_virtualbox::fpu_fop() const
{
	return ext().x87.FOP;
}
std::uint32_t cpu_virtualbox::fpu_ip() const
{
	return ext().x87.FPUIP;
}
std::uint16_t cpu_virtualbox::fpu_cs() const
{
	return ext().x87.CS;
}
std::uint32_t cpu_virtualbox::fpu_dp() const
{
	return ext().x87.FPUDP;
}
std::uint16_t cpu_virtualbox::fpu_ds() const
{
	return ext().x87.DS;
}

std::uint32_t cpu_virtualbox::partial_sse_register(std::uint8_t index, std::uint8_t part_index) const
{
	return ext().x87.aXMM[index].au32[part_index];
}

std::uint32_t cpu_virtualbox::mxcsr() const {
	return (ext().x87.MXCSR);
}
std::uint32_t cpu_virtualbox::mxcsr_mask() const {
	return (ext().x87.MXCSR_MASK);
}

std::uint8_t cpu_virtualbox::mxcsr_ie() const
{
	return (ext().x87.MXCSR & 1);
}
std::uint8_t cpu_virtualbox::mxcsr_de() const
{
	return (ext().x87.MXCSR >> 1) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_ze() const
{
	return (ext().x87.MXCSR >> 2) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_oe() const
{
	return (ext().x87.MXCSR >> 3) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_ue() const
{
	return (ext().x87.MXCSR >> 4) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_pe() const
{
	return (ext().x87.MXCSR >> 5) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_daz() const
{
	return (ext().x87.MXCSR >> 6) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_im() const
{
	return (ext().x87.MXCSR >> 7) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_dm() const
{
	return (ext().x87.MXCSR >> 8) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_zm() const
{
	return (ext().x87.MXCSR >> 9) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_om() const
{
	return (ext().x87.MXCSR >> 10) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_um() const
{
	return (ext().x87.MXCSR >> 11) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_pm() const
{
	return (ext().x87.MXCSR >> 12) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_rc() const
{
	return (ext().x87.MXCSR >> 13) & 3;
}
std::uint8_t cpu_virtualbox::mxcsr_fz() const
{
	return (ext().x87.MXCSR >> 15) & 1;
}
std::uint8_t cpu_virtualbox::mxcsr_mm() const
{
	return (ext().x87.MXCSR >> 17) & 1;
}

std::uint64_t cpu_virtualbox::rflags() const {
	return (context_.base.rflags);
}

//! @return Set if an arithmetic operation generates a carry or a borrow out of the most significant bit of the result.
bool cpu_virtualbox::carry_flag() const
{
	return (context_.base.rflags) & 1;
}

//! @return Set if the least significant byte of the result contains an even number of 1 bits.
bool cpu_virtualbox::parity_flag() const
{
	return (context_.base.rflags) & (1 << 2);
}

//! @return Set if an arithmetic operation generates a carry or a borrow out of bit 3 of the result.
bool cpu_virtualbox::adjust_flag() const
{
	return (context_.base.rflags) & (1 << 4);
}

//! @return Set if the result is zero.
bool cpu_virtualbox::zero_flag() const
{
	return (context_.base.rflags) & (1 << 6);
}

//! @return Set equal to the most-significant bit of the result, which is the sign bit of a signed integer.
bool cpu_virtualbox::sign_flag() const
{
	return (context_.base.rflags) & (1 << 7);
}

//! @return Set if the integer result is too large a positive number of too small a negative number (excluding the
// sign-bit)
//!   to fit in the destination operand.
bool cpu_virtualbox::overflow_flag() const
{
	return (context_.base.rflags) & (1 << 11);
}

bool cpu_virtualbox::directional_flag() const
{
	return (context_.base.rflags) & (1 << 10);
}
bool cpu_virtualbox::resume_flag() const
{
	return (context_.base.rflags) & (1 << 16);
}
bool cpu_virtualbox::trap_flag() const
{
	return (context_.base.rflags) & (1 << 8);
}
bool cpu_virtualbox::interrupt_flag() const
{
	return (context_.base.rflags) & (1 << 9);
}
bool cpu_virtualbox::cpuid_flag() const
{
	return (context_.base.rflags) & (1 << 21);
}
bool cpu_virtualbox::iopl_flag() const
{
	return (context_.base.rflags) & (3 << 12);
}

bool cpu_virtualbox::eflag_reserved_bit1() const
{
	return (context_.base.rflags) & (1 << 1);
}

bool cpu_virtualbox::is_paging_enabled() const
{
	return context_.base.cr0 & 0x80000000;
}
bool cpu_virtualbox::is_pae_enabled() const
{
	return is_paging_enabled() && (context_.base.cr4 & 0x00000020);
}
bool cpu_virtualbox::is_pse_enabled() const
{
	return (context_.base.cr4 & 0x00000010);
}
bool cpu_virtualbox::is_smep_enabled() const
{
	return (context_.base.cr4 & 0x100000);
}
bool cpu_virtualbox::is_pse36_enabled() const
{
	return (context_.base.rdx & 0x20000);
}
bool cpu_virtualbox::is_nx_enabled() const
{
	return (context_.base.msrEFER & 0x800);
}

std::uint64_t cpu_virtualbox::rax() const
{
	return context_.base.rax;
}
std::uint64_t cpu_virtualbox::rbx() const
{
	return context_.base.rbx;
}
std::uint64_t cpu_virtualbox::rcx() const
{
	return context_.base.rcx;
}
std::uint64_t cpu_virtualbox::rdx() const
{
	return context_.base.rdx;
}

std::uint64_t cpu_virtualbox::rsp() const
{
	return context_.base.rsp;
}
std::uint64_t cpu_virtualbox::rbp() const
{
	return context_.base.rbp;
}
std::uint64_t cpu_virtualbox::rsi() const
{
	return context_.base.rsi;
}
std::uint64_t cpu_virtualbox::rdi() const
{
	return context_.base.rdi;
}

std::uint64_t cpu_virtualbox::r8() const {
	return context_.base.r8;
}
std::uint64_t cpu_virtualbox::r9() const {
	return context_.base.r9;
}
std::uint64_t cpu_virtualbox::r10() const {
	return context_.base.r10;
}
std::uint64_t cpu_virtualbox::r11() const {
	return context_.base.r11;
}
std::uint64_t cpu_virtualbox::r12() const {
	return context_.base.r12;
}
std::uint64_t cpu_virtualbox::r13() const {
	return context_.base.r13;
}
std::uint64_t cpu_virtualbox::r14() const {
	return context_.base.r14;
}
std::uint64_t cpu_virtualbox::r15() const {
	return context_.base.r15;
}

std::uint64_t cpu_virtualbox::cr0() const
{
	return context_.base.cr0;
}
std::uint64_t cpu_virtualbox::cr2() const
{
	return context_.base.cr2;
}
std::uint64_t cpu_virtualbox::cr3() const
{
	return context_.base.cr3;
}
std::uint64_t cpu_virtualbox::cr4() const
{
	return context_.base.cr4;
}
std::uint64_t cpu_virtualbox::cr8() const
{
	return tetrane_context_.cr8;
}

std::uint64_t cpu_virtualbox::rip() const
{
	return context_.base.rip;
}

#define GENERATE_DESCRIPTOR_FUNCTIONS(name)					\
	std::uint64_t cpu_virtualbox::name##_base() const {		\
		return context_.base.name.uAddr; 						\
	} 														\
 															\
	std::uint32_t cpu_virtualbox::name##_limit() const {	\
		return context_.base.name.cb; 							\
	}

GENERATE_DESCRIPTOR_FUNCTIONS(gdtr)
GENERATE_DESCRIPTOR_FUNCTIONS(idtr)

#undef GENERATE_DESCRIPTOR_FUNCTIONS

#define GENERATE_SELECTOR_FUNCTIONS(name)					\
	std::uint16_t cpu_virtualbox::name() const {			\
		return context_.base.name.uSel;							\
	}														\
															\
	std::uint64_t cpu_virtualbox::name##_base() const {		\
		return context_.base.name.uBase;							\
	}														\
															\
	std::uint32_t cpu_virtualbox::name##_limit() const {	\
		return context_.base.name.uLimit;						\
	}														\
															\
	std::uint32_t cpu_virtualbox::name##_attr() const {		\
		return context_.base.name.uAttr;							\
	}														\
															\
	std::uint8_t cpu_virtualbox::name##_attr_type() const {	\
		return context_.base.name.attr.u4Type;					\
	}														\
															\
	bool cpu_virtualbox::name##_attr_desc_type() const {	\
		return context_.base.name.attr.u1DescType;				\
	}														\
															\
	std::uint8_t cpu_virtualbox::name##_attr_dpl() const {	\
		return context_.base.name.attr.u2Dpl;					\
	}														\
															\
	bool cpu_virtualbox::name##_attr_present() const {		\
		return context_.base.name.attr.u1Present;				\
	}														\
															\
	bool cpu_virtualbox::name##_attr_available() const {	\
		return context_.base.name.attr.u1Available;				\
	}														\
															\
	bool cpu_virtualbox::name##_attr_long() const {			\
		return context_.base.name.attr.u1Long;					\
	}														\
															\
	bool cpu_virtualbox::name##_attr_def_big() const {		\
		return context_.base.name.attr.u1DefBig;					\
	}														\
															\
	bool cpu_virtualbox::name##_attr_granularity() const {	\
		return context_.base.name.attr.u1Granularity;			\
	}

GENERATE_SELECTOR_FUNCTIONS(ldtr)
GENERATE_SELECTOR_FUNCTIONS(tr)

GENERATE_SELECTOR_FUNCTIONS(cs)
GENERATE_SELECTOR_FUNCTIONS(ds)
GENERATE_SELECTOR_FUNCTIONS(ss)
GENERATE_SELECTOR_FUNCTIONS(gs)
GENERATE_SELECTOR_FUNCTIONS(fs)
GENERATE_SELECTOR_FUNCTIONS(es)

#undef GENERATE_SELECTOR_FUNCTIONS

std::uint64_t cpu_virtualbox::sysenter_eip_r0() const
{
	return context_.base.sysenter.eip;
}
std::uint64_t cpu_virtualbox::sysenter_esp_r0() const
{
	return context_.base.sysenter.esp;
}
std::uint64_t cpu_virtualbox::sysenter_ss_r0() const
{
	return (context_.base.sysenter.cs & 0xFF) + 8;
}
std::uint64_t cpu_virtualbox::sysenter_cs_r0() const
{
	return context_.base.sysenter.cs & 0xFF;
}

std::uint16_t cpu_virtualbox::cs_r3() const
{
	return (sysenter_cs_r0() + 0x10) | 0x3;
}
std::uint16_t cpu_virtualbox::ss_r3() const
{
	return (sysenter_ss_r0() + 0x10) | 0x3;
}
std::uint16_t cpu_virtualbox::ds_r3() const
{
	return ss_r3();
}

std::uint64_t cpu_virtualbox::dr(std::uint8_t reg) const
{
	return reg < sizeof(context_.base.dr) / sizeof(*context_.base.dr) ? context_.base.dr[reg] : 0;
}


std::uint64_t cpu_virtualbox::msrEFER() const
{
	return (context_.base.msrEFER);
}
std::uint64_t cpu_virtualbox::msrSTAR() const
{
	return (context_.base.msrSTAR);
}
std::uint64_t cpu_virtualbox::msrPAT() const
{
	return (context_.base.msrPAT);
}
std::uint64_t cpu_virtualbox::msrLSTAR() const
{
	return (context_.base.msrLSTAR);
}
std::uint64_t cpu_virtualbox::msrCSTAR() const
{
	return (context_.base.msrCSTAR);
}
std::uint64_t cpu_virtualbox::msrSFMASK() const
{
	return (context_.base.msrSFMASK);
}
std::uint64_t cpu_virtualbox::msrKernelGSBase() const
{
	return (context_.base.msrKernelGSBase);
}
std::uint64_t cpu_virtualbox::msrApicBase() const
{
	return (context_.base.msrApicBase);
}
}
}
} // namespace reven::vmghost

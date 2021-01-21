#pragma once
#include <cstdint>

namespace reven {
namespace vmghost {

typedef struct tetrane_cpu_info {
	std::uint64_t cr8{0x0};
} __attribute__((packed)) tetrane_cpu_info;

namespace vbox {

/** DBGCORECOREDESCRIPTOR::u32Magic. */
static constexpr auto DBGFCORE_MAGIC = 0xc01ac0de;

/** DBGCORECOREDESCRIPTOR::u32FmtVersion. */
// for some unknown reason, oracle bumped the version twice
static constexpr auto DBGFCORE_FMT_VERSION_COMPAT = 0x00010004;
static constexpr auto DBGFCORE_FMT_VERSION = 0x00010005;

static constexpr auto TETRANE_CPU_SECTION_NOTE_TYPE = 0xbeef;
static constexpr auto TETRANE_SECTION_MAGIC = 0x62647570634e5652;

typedef __uint128_t uint128_t;

/**
 * 80-bit MMX/FPU register type.
 */
typedef struct X86FPUMMX {
	std::uint8_t reg[10];
} X86FPUMMX;

static_assert(sizeof(X86FPUMMX) == 10, "Invalid X86FPUMMX size");

/** FPU (x87) register. */
typedef union X86FPUREG {
	/** MMX view. */
	std::uint64_t mmx;
	/** FPU view - todo. */
	X86FPUMMX fpu;
	/** 8-bit view. */
	std::uint8_t au8[16];
	/** 16-bit view. */
	std::uint16_t au16[8];
	/** 32-bit view. */
	std::uint32_t au32[4];
	/** 64-bit view. */
	std::uint64_t au64[2];
	/** 128-bit view. (yeah, very helpful) */
	uint128_t au128[1];
} X86FPUREG;

static_assert(sizeof(X86FPUREG) == 16, "Invalid X86FPUREG size");

/**
 * XMM register union.
 */
typedef union X86XMMREG {
	/** XMM Register view *. */
	uint128_t xmm;
	/** 8-bit view. */
	std::uint8_t au8[16];
	/** 16-bit view. */
	std::uint16_t au16[8];
	/** 32-bit view. */
	std::uint32_t au32[4];
	/** 64-bit view. */
	std::uint64_t au64[2];
	/** 128-bit view. (yeah, very helpful) */
	uint128_t au128[1];
} X86XMMREG;

static_assert(sizeof(X86XMMREG) == 16, "Invalid X86XMMREG size");

/**
 * FPU Extended state (aka FXSAVE/FXRSTORE Memory Region).
 */
typedef struct X86FXSTATE {
	/** 0x00 - Control word. */
	std::uint16_t FCW;
	/** 0x02 - Status word. */
	std::uint16_t FSW;
	/** 0x04 - Tag word. (The upper byte is always zero.) */
	std::uint16_t FTW;
	/** 0x06 - Opcode. */
	std::uint16_t FOP;
	/** 0x08 - Instruction pointer. */
	std::uint32_t FPUIP;
	/** 0x0c - Code selector. */
	std::uint16_t CS;
	std::uint16_t Rsrvd1;
	/** 0x10 - Data pointer. */
	std::uint32_t FPUDP;
	/** 0x14 - Data segment */
	std::uint16_t DS;
	/** 0x16 */
	std::uint16_t Rsrvd2;
	/** 0x18 */
	std::uint32_t MXCSR;
	/** 0x1c */
	std::uint32_t MXCSR_MASK;
	/** 0x20 - FPU registers. */
	X86FPUREG aRegs[8];
	/** 0xA0 - XMM registers - 8 registers in 32 bits mode, 16 in long mode. */
	X86XMMREG aXMM[16];
	/* - offset 416 - */
	std::uint32_t au32RsrvdRest[(464 - 416) / sizeof(std::uint32_t)];
	/* - offset 464 - Software usable reserved bits. */
	std::uint32_t au32RsrvdForSoftware[(512 - 464) / sizeof(std::uint32_t)];
} X86FXSTATE;

static_assert(sizeof(X86FXSTATE) == 512, "Invalid X86FXSTATE size");

/**
 * XSAVE header.
 */
typedef struct X86XSAVEHDR {
	/** XTATE_BV - Bitmap indicating whether a component is in the state. */
	std::uint64_t bmXState;
	/** XCOMP_BC - Bitmap used by instructions applying structure compaction. */
	std::uint64_t bmXComp;
	/** Reserved for furture extensions, probably MBZ. */
	std::uint64_t au64Reserved[6];
} X86XSAVEHDR;

static_assert(sizeof(X86XSAVEHDR) == 64, "Invalid X86XSAVEHDR size");

/**
 * x86 FPU/SSE/AVX/XXXX state.
 *
 * Please bump DBGFCORE_FMT_VERSION by 1 in dbgfcorefmt.h if you make any
 * changes to this structure.
 */
typedef struct X86XSAVEAREA {
	/** The x87 and SSE region (or legacy region if you like).  */
	X86FXSTATE x87;
	/** The XSAVE header. */
	X86XSAVEHDR Hdr;
	/** Beyond the header, there isn't really a fixed layout, but we can
	   generally assume the YMM (AVX) register extensions are present and
	   follows immediately. */
	union {
		/** This is a typical layout on intel CPUs (good for debuggers). */
		// struct
		//{
		//    X86XSAVEYMMHI       YmmHi;
		//    X86XSAVEBNDREGS     BndRegs;
		//    X86XSAVEBNDCFG      BndCfg;
		//    std::uint8_t        abFudgeToMatchDocs[0xB0];
		//    X86XSAVEOPMASK      Opmask;
		//    X86XSAVEZMMHI256    ZmmHi256;
		//    X86XSAVEZMM16HI     Zmm16Hi;
		//} Intel;

		/** This is a typical layout on AMD Bulldozer type CPUs (good for debuggers). */
		// struct
		//{
		//    X86XSAVEYMMHI       YmmHi;
		//    X86XSAVELWP         Lwp;
		//} AmdBd;

		/** To enbling static deployments that have a reasonable chance of working for
		 * the next 3-6 CPU generations without running short on space, we allocate a
		 * lot of extra space here, making the structure a round 8KB in size.  This
		 * leaves us 7616 bytes for extended state.  The skylake xeons are likely to use
		 * 2112 of these, leaving us with 5504 bytes for future Intel generations. */
		std::uint8_t ab[8192 - 512 - 64];
	} u;
} X86XSAVEAREA;

static_assert(sizeof(X86XSAVEAREA) == 8192, "Invalid X86XSAVEAREA size");

/**
 * An x86 segment selector.
 */
typedef struct DBGFCORESEL {
	std::uint64_t uBase;
	std::uint32_t uLimit;
	union {
		std::uint32_t uAttr;
		struct {
			/** 00 - Segment Type. */
			unsigned u4Type : 4;
			/** 04 - Descriptor Type. System(=0) or code/data selector */
			unsigned u1DescType : 1;
			/** 05 - Descriptor Privilege level. */
			unsigned u2Dpl : 2;
			/** 07 - Flags selector present(=1) or not. */
			unsigned u1Present : 1;
			/** 08 - Segment limit 16-19. */
			unsigned u4LimitHigh : 4;
			/** 0c - Available for system software. */
			unsigned u1Available : 1;
			/** 0d - 32 bits mode: Reserved - 0, long mode: Long Attribute Bit. */
			unsigned u1Long : 1;
			/** 0e - This flags meaning depends on the segment type. Try make sense out
			 * of the intel manual yourself.  */
			unsigned u1DefBig : 1;
			/** 0f - Granularity of the limit. If set 4KB granularity is used, if
			 * clear byte. */
			unsigned u1Granularity : 1;
			/** 10 - "Unusable" selector, special Intel (VT-x only?) bit. */
			unsigned u1Unusable : 1;
		} attr;
	};
	std::uint16_t uSel;
	std::uint16_t uReserved0;
	std::uint32_t uReserved1;
} VBOXX86SEL;

/**
 * A gdtr/ldtr descriptor.
 */
typedef struct DBGFCOREXDTR {
	std::uint64_t uAddr;
	std::uint32_t cb;
	std::uint32_t uReserved0;
} DBGFXDTR;

/**
 * A simpler to parse CPU dump than CPUMCTX.
 *
 * Please bump DBGFCORE_FMT_VERSION by 1 if you make any changes to this
 * structure.
 */
typedef struct DBGFCORECPU {
	std::uint64_t rax;
	std::uint64_t rbx;
	std::uint64_t rcx;
	std::uint64_t rdx;
	std::uint64_t rsi;
	std::uint64_t rdi;
	std::uint64_t r8;
	std::uint64_t r9;
	std::uint64_t r10;
	std::uint64_t r11;
	std::uint64_t r12;
	std::uint64_t r13;
	std::uint64_t r14;
	std::uint64_t r15;
	std::uint64_t rip;
	std::uint64_t rsp;
	std::uint64_t rbp;
	std::uint64_t rflags;
	DBGFCORESEL cs;
	DBGFCORESEL ds;
	DBGFCORESEL es;
	DBGFCORESEL fs;
	DBGFCORESEL gs;
	DBGFCORESEL ss;
	std::uint64_t cr0;
	std::uint64_t cr2;
	std::uint64_t cr3;
	std::uint64_t cr4;
	std::uint64_t dr[8];
	DBGFCOREXDTR gdtr;
	DBGFCOREXDTR idtr;
	VBOXX86SEL ldtr;
	VBOXX86SEL tr;
	struct {
		std::uint64_t cs;
		std::uint64_t eip;
		std::uint64_t esp;
	} sysenter;
	std::uint64_t msrEFER;
	std::uint64_t msrSTAR;
	std::uint64_t msrPAT;
	std::uint64_t msrLSTAR;
	std::uint64_t msrCSTAR;
	std::uint64_t msrSFMASK;
	std::uint64_t msrKernelGSBase;
	std::uint64_t msrApicBase;
	std::uint64_t aXcr[2];
	X86XSAVEAREA ext;
} DBGFCORECPU;

static_assert(sizeof(DBGFCORECPU) == 8768, "Invalid DBGFCORECPU size");

/**
 * The DBGF Core descriptor.
 */
typedef struct DBGFCOREDESCRIPTOR {
	/** The core file magic (DBGFCORE_MAGIC) */
	std::uint32_t u32Magic;
	/** The core file format version (DBGFCORE_FMT_VERSION). */
	std::uint32_t u32FmtVersion;
	/** Size of this structure (sizeof(DBGFCOREDESCRIPTOR)). */
	std::uint32_t cbSelf;
	/** VirtualBox version. */
	std::uint32_t u32VBoxVersion;
	/** VirtualBox revision. */
	std::uint32_t u32VBoxRevision;
	/** Number of CPUs. */
	std::uint32_t cCpus;
} DBGFCOREDESCRIPTOR;

static_assert(sizeof(DBGFCOREDESCRIPTOR) == 24, "Invalid DBGFCOREDESCRIPTORs size");

/**
 * The Elf note type of vbox core.
 */
static constexpr auto NT_VBOXCORE = 0xb00;
static constexpr auto NT_VBOXCPU = 0xb01;

/**
 * The Elf note names of vbox core.
 */
static constexpr auto NN_VBOXCORE = "VBCORE";
static constexpr auto NN_VBOXCPU = "VBCPU";
}
}
} // namespace reven::vmghost::vbox

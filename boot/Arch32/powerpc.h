#ifndef _POWERPC_H_
#define _POWERPC_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Processor Version Register (PVR) values
#define PVR_601       0x00010000
#define PVR_603       0x00030000
#define PVR_604       0x00040000
#define PVR_750       0x00080000
#define PVR_7400      0x000C0000
#define PVR_970       0x00390002
#define PVR_POWER8    0x004B0000
#define PVR_POWER9    0x004E0000

// MSR (Machine State Register) bits
#define MSR_LE     (1ULL << 0)   // Little-endian mode
#define MSR_RI     (1ULL << 1)   // Recoverable interrupt
#define MSR_PM     (1ULL << 2)   // Power management
#define MSR_EE     (1ULL << 15)  // External interrupt enable
#define MSR_PR     (1ULL << 17)  // Privilege level (0=supervisor, 1=user)
#define MSR_FP     (1ULL << 18)  // Floating-point available
#define MSR_VEC    (1ULL << 25)  // AltiVec available
#define MSR_VSX    (1ULL << 26)  // VSX available
#define MSR_64BIT  (1ULL << 32)  // 64-bit mode

// SPR (Special Purpose Register) numbers
enum {
    SPR_XER = 1, SPR_LR, SPR_CTR, SPR_DSISR, SPR_DAR, SPR_DEC, SPR_SDR1 = 25,
    SPR_SRR0 = 26, SPR_SRR1, SPR_CFAR = 28, SPR_HSRR0 = 314, SPR_HSRR1 = 315,
    SPR_SPRG0 = 272, SPR_SPRG1, SPR_SPRG2, SPR_SPRG3, SPR_SPRG4, SPR_SPRG5,
    SPR_SPRG6, SPR_SPRG7, SPR_IVPR = 63, SPR_PVR = 287, SPR_PIR = 286,
    SPR_MMUCR = 370, SPR_MMUCFG = 1015, SPR_MMUCSR0 = 1016, SPR_MMUCSR1 = 1017,
    SPR_TBRL = 268, SPR_TBRU = 269, SPR_TBWL = 284, SPR_TBWU = 285,
    SPR_IVOR0 = 400, SPR_IVOR1, SPR_IVOR2, SPR_IVOR3, SPR_IVOR4, SPR_IVOR5,
    SPR_IVOR6, SPR_IVOR7, SPR_IVOR8, SPR_IVOR9, SPR_IVOR10, SPR_IVOR11,
    SPR_IVOR12, SPR_IVOR13, SPR_IVOR14, SPR_IVOR15, SPR_IVOR32 = 528,
    SPR_IVOR33, SPR_IVOR34, SPR_IVOR35, SPR_IVOR36, SPR_IVOR37, SPR_IVOR38,
    SPR_IVOR39, SPR_IVOR40, SPR_IVOR41, SPR_IVOR42, SPR_IVOR43, SPR_IVOR44,
    SPR_IVOR45, SPR_IVOR46, SPR_IVOR47, SPR_IVOR48, SPR_IVOR49, SPR_IVOR50,
    SPR_IVOR51, SPR_IVOR52, SPR_IVOR53, SPR_IVOR54, SPR_IVOR55, SPR_IVOR56,
    SPR_IVOR57, SPR_IVOR58, SPR_IVOR59, SPR_IVOR60, SPR_IVOR61, SPR_IVOR62,
    SPR_IVOR63 = 591
};

// Common PowerPC register definitions
#define PPC_MSR_LE    (1ULL << 0)  // Little-endian mode
#define PPC_MSR_RI    (1ULL << 1)  // Recoverable interrupt
#define PPC_MSR_PM    (1ULL << 2)  // Power management
#define PPC_MSR_64BIT (1ULL << 32) // 64-bit mode

// SPR (Special Purpose Register) numbers
#define PPC_SPR_SRR0   26  // Save/Restore Register 0
#define PPC_SPR_SRR1   27  // Save/Restore Register 1
#define PPC_SPR_IVPR   63  // Interrupt Vector Prefix Register
#define PPC_SPR_IVOR0  400 // Interrupt Vector Offset Register 0
#define PPC_SPR_IVOR1  401 // ...
#define PPC_SPR_IVOR2  402
#define PPC_SPR_IVOR3  403
#define PPC_SPR_IVOR4  404

// Exception vector offsets
#define PPC_EXC_SYSTEM_RESET  0x0100
#define PPC_EXC_MACHINE_CHECK 0x0200
#define PPC_EXC_DSI           0x0300
#define PPC_EXC_ISI           0x0400
#define PPC_EXC_EXTERNAL      0x0500

// CPU feature flags
#define PPC_FEATURE_64        0x40000000
#define PPC_FEATURE_HAS_ALTIVEC 0x10000000
#define PPC_FEATURE_HAS_VSX    0x00000080
#define PPC_FEATURE_ARCH_2_05  0x00001000  // ISA 2.05 (POWER5+)
#define PPC_FEATURE_ARCH_2_06  0x00000100  // ISA 2.06 (POWER7+)
#define PPC_FEATURE_ARCH_2_07  0x00000080  // ISA 2.07 (POWER8+)

// MMU definitions
#define PPC_TLB_VALID  0x00000001
#define PPC_TLB_WRITE  0x00000002
#define PPC_TLB_EXEC   0x00000004
#define PPC_TLB_USER   0x00000008
#define PPC_TLB_GUARD  0x00000010
#define PPC_TLB_COHERENT 0x00000020

// CPU Identification
struct ppc_cpu_info {
    uint32_t pvr;
    uint32_t version;
    uint32_t revision;
    uint32_t mmu_type;
    uint32_t cpu_features;
    uint32_t icache_size;
    uint32_t dcache_size;
    uint32_t l1_cache_line_size;
    uint32_t l2_cache_size;
    uint32_t l3_cache_size;
    bool has_fpu;
    bool has_altivec;
    bool has_vsx;
    bool has_smt;
    bool is_64bit;
};

// MMU Context
struct ppc_mmu_context {
    uint64_t sdr1;
    uint64_t vsid[16];
    uint32_t sr[16];
    uint64_t htaborg;
    uint32_t htabmask;
    uint32_t vsid_multiplier;
};

// Exception Frame
struct ppc_exception_frame {
    uint64_t gpr[32];
    uint64_t cr;
    uint64_t xer;
    uint64_t lr;
    uint64_t ctr;
    uint64_t srr0;
    uint64_t srr1;
    uint64_t dar;
    uint64_t dsisr;
    uint64_t cfar;
    uint64_t hssrr0;
    uint64_t hssrr1;
    uint64_t hdar;
    uint64_t hdsisr;
    uint64_t hsrr0;
    uint64_t hsrr1;
};

// Function prototypes
void ppc_early_init(void);
void ppc_mmu_init(void);
void ppc_cache_init(void);
void ppc_timer_init(void);
void ppc_irq_init(void);
void ppc_configure_smt(bool enable);
void ppc_setup_tlb(uint64_t ea, uint64_t pa, uint32_t size, uint32_t wimg, uint32_t perms);
void ppc_invalidate_tlb(uint64_t ea);
void ppc_flush_dcache_range(void *addr, size_t size);
void ppc_invalidate_icache_range(void *addr, size_t size);
void ppc_sync_icache(void *addr, size_t size);
void ppc_set_dec(uint32_t val);
uint32_t ppc_get_dec(void);
uint64_t ppc_get_timebase(void);
void ppc_enable_fp(void);
void ppc_enable_altivec(void);
void ppc_enable_vsx(void);
void ppc_enable_hv(void);
void ppc_enable_isel(void);
void ppc_enable_doorbell(void);
void ppc_configure_power_save(uint32_t psscr_val);
void ppc_enter_nap(void);
void ppc_enter_sleep(void);
void ppc_enter_winkle(void);
void ppc_enter_stop(uin32_t stop_level);
void ppc_handle_thermal(void);
void ppc_handle_perf_mon(void);
void ppc_handle_debug(void);
void ppc_handle_hmi(void);
void ppc_handle_hmi_early(void);
void ppc_handle_machine_check_early(struct ppc_exception_frame *frame);
void ppc_handle_machine_check(struct ppc_exception_frame *frame);
void ppc_handle_altivec_unavailable(struct ppc_exception_frame *frame);
void ppc_handle_vsx_unavailable(struct ppc_exception_frame *frame);
void ppc_handle_facility_unavailable(struct ppc_exception_frame *frame);
void ppc_handle_h_facility_unavailable(struct ppc_exception_frame *frame);
void ppc_handle_alignment(struct ppc_exception_frame *frame);
void ppc_handle_program(struct ppc_exception_frame *frame);
void ppc_handle_fp_unavailable(struct ppc_exception_frame *frame);
void ppc_handle_decrementer(struct ppc_exception_frame *frame);
void ppc_handle_system_call(struct ppc_exception_frame *frame);
void ppc_handle_trace(struct ppc_exception_frame *frame);
void ppc_handle_perf_mon(struct ppc_exception_frame *frame);
void ppc_handle_altivec_assist(struct ppc_exception_frame *frame);
void ppc_handle_thermal(struct ppc_exception_frame *frame);
void ppc_handle_h_doorbell(struct ppc_exception_frame *frame);
void ppc_handle_h_virt(struct ppc_exception_frame *frame);
void ppc_handle_h_doorbell(struct ppc_exception_frame *frame);
void ppc_handle_hmi(struct ppc_exception_frame *frame);

// Low-level CPU operations
static inline uint64_t mfspr(uint32_t spr) {
    uint64_t val;
    asm volatile("mfspr %0, %1" : "=r"(val) : "i"(spr));
    return val;
}

static inline void mtspr(uint32_t spr, uint64_t val) {
    asm volatile("mtspr %0, %1" : : "i"(spr), "r"(val));
}

static inline uint64_t mfmsr(void) {
    uint64_t msr;
    asm volatile("mfmsr %0" : "=r"(msr));
    return msr;
}

static inline void mtmsr(uint64_t msr) {
    asm volatile("mtmsrd %0" : : "r"(msr) : "memory");
}

static inline void mtmsr_ee(uint64_t msr) {
    asm volatile("mtmsrd %0, 1" : : "r"(msr) : "memory");
}

static inline uint64_t mftb(void) {
    uint32_t tb_upper, tb_lower, tmp;
    do {
        asm volatile("mftbu %0" : "=r"(tb_upper));
        asm volatile("mftb  %0" : "=r"(tb_lower));
        asm volatile("mftbu %0" : "=r"(tmp));
    } while (tmp != tb_upper);
    return ((uint64_t)tb_upper << 32) | tb_lower;
}

static inline void tlbiel(uint64_t rb) {
    asm volatile("tlbiel %0" : : "r"(rb) : "memory");
}

static inline void tlbie(uint64_t rb, uint32_t rs) {
    asm volatile("tlbie %0, %1" : : "r"(rb), "r"(rs) : "memory");
}

static inline void tlbivax(uint64_t a, uint64_t b) {
    asm volatile("tlbivax %0, %1" : : "r"(a), "r"(b) : "memory");
}

static inline void tlbsx(uint64_t a, uint64_t b) {
    asm volatile("tlbsx. %0, %1, %2" : "=r"(a) : "r"(b), "r"(a) : "memory");
}

static inline void tlbwe(uint32_t a, uint32_t b, uint32_t c) {
    asm volatile("tlbwe %0, %1, %2" : : "r"(a), "r"(b), "r"(c) : "memory");
}

static inline void tlbivax_all(void) {
    asm volatile("tlbivax 0, 0" : : : "memory");
}

static inline void tlbiel_all(void) {
    asm volatile("tlbiel 0" : : : "memory");
}

static inline void tlbsync(void) {
    asm volatile("tlbsync" : : : "memory");
}

static inline void eieio(void) {
    asm volatile("eieio" : : : "memory");
}

static inline void isync(void) {
    asm volatile("isync" : : : "memory");
}

static inline void sync(void) {
    asm volatile("sync" : : : "memory");
}

static inline void msync(void) {
    asm volatile("msync" : : : "memory");
}

static inline void lwsync(void) {
    asm volatile("lwsync" : : : "memory");
}

static inline void ppc_ppc_stop(void) {
    asm volatile("stop" : : : "memory");
}

static inline void ppc_ppc_pause(void) {
    asm volatile("or 27,27,27");
}

static inline void ppc_ppc_mdoio(void) {
    asm volatile("mdoio");
}

static inline void ppc_ppc_mdoom(void) {
    asm volatile("mdoom");
}

static inline void ppc_ppc_mtmsr(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val | MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val & ~MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_disable(uint64_t val) {
    asm volatile("mtmsrd %0,0" : : "r"(val & ~MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_enable(uint64_t val) {
    asm volatile("mtmsrd %0,0" : : "r"(val | MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_disable(uint64_t val) {
    asm volatile("mtmsrd %0,0" : : "r"(val & ~(MSR_EE | MSR_RI)) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_enable_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,0" : : "r"(val | MSR_EE | MSR_RI) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_enable(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val | MSR_EE | MSR_RI) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_disable(uint64_t val) {
    asm volatile("mtmsrd %0,0" : : "r"(val & ~(MSR_EE | MSR_RI)) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_enable(uint64_t val) {
    asm volatile("mtmsrd %0,0" : : "r"(val | MSR_EE | MSR_RI) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val & ~MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val & ~MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val | MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val & ~MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val & ~MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val | MSR_EE) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

static inline void ppc_ppc_mtmsr_ee_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore_irqsoff_restore_irqson_restore(uint64_t val) {
    asm volatile("mtmsrd %0,1" : : "r"(val) : "memory");
}

// Boot protocol support
struct device_tree_node;

int ppc_boot_linux(void *kernel, size_t size, const char *cmdline);
int ppc_boot_elf(void *elf);
int ppc_load_device_tree(struct device_tree_node **dt);
void ppc_set_bootargs(const char *cmdline);

// Cache control
void ppc_invalidate_icache(void);
void ppc_invalidate_dcache(void);
void ppc_flush_dcache(void);

// Timebase functions
static inline uint64_t ppc_mftb(void) {
    uint32_t upper, lower, tmp;
    do {
        asm volatile ("mftbu %0" : "=r" (upper));
        asm volatile ("mftb  %0" : "=r" (lower));
        asm volatile ("mftbu %0" : "=r" (tmp));
    } while (tmp != upper);
    return ((uint64_t)upper << 32) | lower;
}

// Memory barriers
#define ppc_sync()      asm volatile ("sync" : : : "memory")
#define ppc_isync()     asm volatile ("isync" : : : "memory")
#define ppc_lwsync()    asm volatile ("lwsync" : : : "memory")
#define ppc_eieio()     asm volatile ("eieio" : : : "memory")

// Read/Write special purpose registers
static inline uint64_t mfspr(int spr) {
    uint64_t val;
    asm volatile ("mfspr %0, %1" : "=r" (val) : "i" (spr));
    return val;
}

static inline void mtspr(int spr, uint64_t val) {
    asm volatile ("mtspr %0, %1" : : "i" (spr), "r" (val));
}

// Cache operations
static inline void icbi(void *addr) {
    asm volatile ("icbi 0, %0" : : "r" (addr) : "memory");
}

static inline void dcbf(void *addr) {
    asm volatile ("dcbf 0, %0" : : "r" (addr) : "memory");
}

static inline void dcbst(void *addr) {
    asm volatile ("dcbst 0, %0" : : "r" (addr) : "memory");
}

static inline void dcbz(void *addr) {
    asm volatile ("dcbz 0, %0" : : "r" (addr) : "memory");
}

#endif /* _POWERPC_H_ */

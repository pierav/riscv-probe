#include "femto.h"

#ifdef __riscv
#include "arch/riscv/encoding.h"
#include "arch/riscv/machine.h"
#include "arch/riscv/trap.h"
#endif

#define MTIMECMP_BASE	0x4000
#define MTIME_BASE	0xbff8

void machine_handler(uintptr_t* regs, uintptr_t mcause, uintptr_t mepc)
{
#if 0
	printf("Caught a trap @0x%p\n", mepc);
#endif
	uint64_t time = *(volatile uint64_t *) (CLINT_BASE + MTIME_BASE);
	*(volatile uint64_t *) (CLINT_BASE + MTIMECMP_BASE) = time + 1000000;
}

int main(int argc, char **argv)
{
	/*
	 * Set up PMP (Physical Memory Protection)
	 *
	 * PMP is optional; bit if implemented enforcement is mandatory
	 * and user-mode is prevented access to all memory by default.
	 * We should change this code to restrict user mode to its .text
	 * .rodata, .data, .bss and devices.
	 */
#ifdef __riscv
	if (pmp_entry_count() > 0) {
		pmp_allow_all();
	}
	set_trap_fn(machine_handler);
	set_csr(mie, MIP_MTIP);
	set_csr(mstatus, MSTATUS_MIE);
	uint64_t time = *(volatile uint64_t *) (CLINT_BASE + MTIME_BASE);
	*(volatile uint64_t *) (CLINT_BASE + MTIMECMP_BASE) = time + 1000000;
        uint64_t status;
        status = read_csr(mstatus);
        printf("mstatus=0x%016x\n", status);
	mode_set_and_continue(PRV_S);
	puts("riscv-supervisor-mode");
        status = read_csr(sstatus);
        printf("sstatus=0x%016x\n", status);
        extern uint64_t pgdir[4096];
        uint64_t satp_value = (0x8ul << 60) | ((uint64_t)pgdir << 12);
        /* 1:1 gigapage mapping =>
           0x8000 0000 -> 0xbfff ffff mapped to 0x8000 0000 -> 0xbfff ffff
           VPN[2]    VPN[1]    VPN[0]    page  offset
           000000010 000000000 000000000 000000000000 -> 000000010 111111111 111111111 111111111111
           page directory entry : 2
           PPN[2]    PPN[1]    PPN[0]    page  offset
           000000010 000000000 000000000 000000000000 -> 000000010 111111111 111111111 111111111111
        */
        pgdir[2] = (2 << 28) | 0xf;
        write_csr(satp, satp_value);

	while (1) {
		asm("wfi");
		putchar('.');
		status = read_csr(sstatus);
#if 0
		printf("sstatus=0x%016x\n", status);
#endif
	}
#else
	puts("architecture-not-supported");
	exit(1);
#endif
}

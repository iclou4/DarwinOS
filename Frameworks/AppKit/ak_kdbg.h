#ifndef AK_KDBG_H
#define AK_KDBG_H

/* Raw kernel-log probe: no Foundation involved.  Length is a compile-time
 * constant (fixed literal) so broken constant strings can never hang it. */
static inline void
ak_kdbg_raw(const char *msg, unsigned long len)
{
    register long rax __asm__("rax") = (0x2000000L | 535L);
    register long rdi __asm__("rdi") = (long)msg;
    register long rsi __asm__("rsi") = 5;
    register long rdx __asm__("rdx") = 5;
    register long r10 __asm__("r10") = (long)len;
    __asm__ volatile("syscall"
        : "+r"(rax)
        : "r"(rdi), "r"(rsi), "r"(rdx), "r"(r10)
        : "rcx", "r11", "memory");
}

#define AK_KDBG(lit) ak_kdbg_raw(lit, sizeof(lit) - 1)

/* numeric probe (decimal) — implementation lives in nib.subproj/NSIBObjectData.m */
extern void nib_kdbg_num(const char *tag, unsigned long v);
#define AK_KDBGN(tag, v) nib_kdbg_num(tag, (unsigned long)(v))

#endif

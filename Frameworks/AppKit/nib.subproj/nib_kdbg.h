#ifndef NIB_KDBG_H
#define NIB_KDBG_H

/* Write a marker straight into the kernel log, bypassing Foundation.
 * Layout matches CoreServices/WindowServer2: class (5,5) of syscall 535.
 * The length is a compile-time constant: the guest runtime still has broken
 * constant strings, and strlen() on one of those never returns. */
static inline void
nib_kdbg_raw(const char *msg, unsigned long len)
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

#define NIB_KDBG(lit) nib_kdbg_raw(lit, sizeof(lit) - 1)

#endif

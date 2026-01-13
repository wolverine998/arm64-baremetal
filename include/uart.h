#ifndef __UART__
#define __UART__

#include <stdint.h>
void uart_putc(char c);
void uart_puts(const char *s);
void uart_puts_old(const char *s);
void uart_hex(uint64_t value);

// el1 uart
void kernel_putc(char c);
void kernel_puts(const char *s);
void kernel_puts_old(const char *s);
void kernel_hex(uint64_t value);
void kernel_printf(const char *fmt, ...);

// Secure uart
void seeos_putc(char c);
void seeos_puts_atomic(const char *s);
void seeos_puts(const char *s);
void seeos_hex(uint64_t value);
void seeos_printf(const char *fmt, ...);

#define UART_VA 0x0600000
#define UART_BASE 0x09000000
#define SECURE_UART1 0x09040000

#endif

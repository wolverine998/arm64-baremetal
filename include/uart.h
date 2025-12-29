#ifndef __UART__
#define __UART__

void uart_putc(char c);
void uart_puts(const char *s);

#define UART_VA 0x0600000
#define UART_BASE 0x09000000

#endif

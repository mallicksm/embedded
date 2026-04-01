#pragma once

void uart_putc(char);
char uart_getc(void);
void uart_puts(const char*);
int uart_getc_nonblock(void);

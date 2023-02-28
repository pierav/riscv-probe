// See LICENSE for license details.

#include "femto.h"

/**************************   UART   ************************/

#include "cva6_opensbi_uart.h"

#define ARIANE_UART_ADDR			0x10000000
#define ARIANE_UART_FREQ		    50000000
#define ARIANE_UART_BAUDRATE		115200
#define ARIANE_UART_REG_SHIFT		2
#define ARIANE_UART_REG_WIDTH	    4

void console_cva6_init(){
    uart8250_init(ARIANE_UART_ADDR,
                   ARIANE_UART_FREQ,
                   ARIANE_UART_BAUDRATE,
                   ARIANE_UART_REG_SHIFT,
                   ARIANE_UART_REG_WIDTH);
}

int console_cva6_getchar(){
    return uart8250_getc();
}

int console_cva6_putchar(int ch){
    uart8250_putc(ch);
    return 0;
}

console_device_t console_cva6 = {
    console_cva6_init,
    console_cva6_getchar,
    console_cva6_putchar
};

/************************** POWEROFF ************************/

// Custom poweroff

extern uint64_t tohost;
extern uint64_t fromhost;

void __attribute__((noreturn, noinline)) pass () {
   tohost = 1; // FESVR exit OK
   while(1);
}


void __attribute__((noreturn, noinline)) fail (uintptr_t code) {
    tohost = (code << 1) | 1; // FESVR exit
     while(1);
}

void poweroff_good_trab_bad_trap_init(void){
    /* Do nothing */
}

void poweroff_good_trab_bad_trap_poweroff(int code){
    if(code >> 1) {
        fail(code >> 1);
    } else {
        pass();
    }
}

poweroff_device_t poweroff_good_trab_bad_trap = {
    poweroff_good_trab_bad_trap_init,
    poweroff_good_trab_bad_trap_poweroff
};


/**************************  SETUP   ************************/

auxval_t __auxv[] = {
    { 0, 0 }
};

void arch_setup()
{
    register_console(&console_cva6);
    register_poweroff(&poweroff_good_trab_bad_trap);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <irq.h>
#include <uart.h>
#include <console.h>
#include <generated/csr.h>
#include <generated/soc.h>
#include <system.h>
#include "lib/aht10.h"
#include "lib/rfm96.h"

static char *readstr(void)
{
    char c[2];
    static char s[64];
    static int ptr = 0;

    if(readchar_nonblock()) {
        c[0] = readchar();
        c[1] = 0;
        switch(c[0]) {
            case 0x7f:
            case 0x08:
                if(ptr > 0) {
                    ptr--;
                    putsnonl("\x08 \x08");
                }
                break;
            case 0x07:
                break;
            case '\r':
            case '\n':
                s[ptr] = 0x00;
                putsnonl("\n");
                ptr = 0;
                return s;
            default:
                if(ptr >= (sizeof(s) - 1))
                    break;
                putsnonl(c);
                s[ptr] = c[0];
                ptr++;
                break;
        }
    }
    return NULL;
}

static char *get_token(char **str)
{
    char *c, *d;

    c = (char *)strchr(*str, ' ');
    if(c == NULL) {
        d = *str;
        *str = *str+strlen(*str);
        return d;
    }
    *c = 0;
    d = *str;
    *str = c+1;
    return d;
}

static void prompt(void)
{
    printf("RUNTIME>");
}

static void help(void)
{
    puts("Available commands:");
    puts("help                            - this command");
    puts("reboot                          - reboot CPU");
    puts("led                             - led test");
    puts("aht10                           - aht10 send data");
}

static void reboot(void)
{
    ctrl_reset_write(1);
}

static void toggle_led(void)
{
    int i;
    printf("invertendo led...\n");
    i = leds_out_read();
    leds_out_write(!i);
}



static void send_aht10_data(void) {
    aht10 sensor_data; 
    printf("Lendo dados do AHT10\n");

    if (aht10_get_data(&sensor_data)) {
        printf("  Temperatura: %d.%02d C\n", sensor_data.temperatura/100, abs(sensor_data.temperatura) % 100);
        printf("  Umidade: %d.%02d %%\n", sensor_data.umidade/100, abs(sensor_data.umidade) % 100);

        // Chama a função da biblioteca LoRa
        if (!rfm96_send((uint8_t*)&sensor_data, sizeof(aht10))) {
             printf("Erro durante o envio\n");
        }
    } else {
        printf("Erro ao ler dados do AHT10\n");
    }
}

static void console_service(void) {
    char *str;
    char *token;

    str = readstr();
    if(str == NULL) return;
    token = get_token(&str);
    if(strcmp(token, "help") == 0)
        help();
    else if(strcmp(token, "reboot") == 0)
        reboot();
    else if(strcmp(token, "led") == 0)
        toggle_led();
    else if(strcmp(token, "aht10") == 0)
        send_aht10_data();
    prompt();
}

int main(void) {
#ifdef CONFIG_CPU_HAS_INTERRUPT
    irq_setmask(0);
    irq_setie(1);
#endif
    uart_init();

// Inicializa timer0 
#ifdef CSR_TIMER0_BASE
    timer0_en_write(0);
    timer0_reload_write(0);
    timer0_load_write(CONFIG_CLOCK_FREQUENCY / 1000000); 
    timer0_en_write(1);
    timer0_update_value_write(1);
#endif

    busy_wait_us(500000);

    printf("\n===============================================\n");
    printf(  "   Transmissão de temperatura e umidade \n");

    uart_init();
    i2c_init();
    aht10_init();


    if (!rfm96_init()) {
        printf("FALHA NA INICIALIZAÇÂO DO LORA\n");
        while(1);
    }

    help();
    prompt();

    while (1) {
        console_service();
    }

    return 0;
}

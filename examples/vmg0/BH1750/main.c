#include "bflb_mtimer.h"
#include "board.h"
#include "bh1750.h"


int main(void)
{
    float lux=0.0;
    board_init();
    board_i2c0_gpio_init();

    BH1750_Init();

    while (1) {
        lux = BH1750_GetData();
        printf("lux:%.2f\r\n", lux);
        bflb_mtimer_delay_ms(1000);
    }
}

#include "bflb_mtimer.h"
#include "board.h"
#include "lcd_conf_user.h"
#include "lcd.h"

#define DBG_TAG "MAIN"
#include "log.h"

void drawVeriMake(uint8_t x, uint8_t y, uint8_t size)
{
	uint8_t middle = size / 2;
	uint8_t x1 = x + middle;
	uint8_t y1 = y + middle;
	uint8_t x2 = x + size;
	uint8_t y2 = y + size;
	lcd_draw_rectangle(x, y, x2, y2,0xFFFF);
	lcd_draw_rectangle(x, y1, x1, y2,0xFFFF);
	lcd_draw_line(x, y1, x1, y,0xFFFF);
	lcd_draw_line(x1, y1, x2, y,0xFFFF);
	lcd_draw_line(x1, y1, x2, y2,0xFFFF);
}

int main(void)
{
    board_init();
    lcd_init();
    lcd_clear(0x0000);
    lcd_draw_str_ascii16(50,100,1000000,100,"Hello Henry Wang!",17);
	//第三四个变量像是颜色，三是字体颜色，四是背景颜色
    //drawVeriMake(100,10,20);

    while (1) {
        // bflb_mtimer_delay_ms(1000);
    }
}

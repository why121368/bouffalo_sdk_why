#include "bflb_mtimer.h"
#include "board.h"
#include "touch_conf_user.h"
#include "touch.h"

touch_coord_t touch_max_point = {
        .coord_x = 240,
        .coord_y = 320,
    };

int main(void)
{
    board_init();
    bflb_mtimer_delay_ms(50);
    touch_init(&touch_max_point);

    static int16_t last_x = 0;
    static int16_t last_y = 0;
    uint8_t point_num = 0;
    touch_coord_t touch_coord;

    while (1) {
        touch_read(&point_num, &touch_coord, 1);
        if (point_num) {
            last_x = touch_coord.coord_x;
            last_y = touch_coord.coord_y<=160?touch_coord.coord_y+160:touch_coord.coord_y-160;
            printf("x=%d, y=%d\r\n",last_x,last_y);
        } 
        // bflb_mtimer_delay_ms(1000);
    }
}

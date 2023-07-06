#include "bflb_mtimer.h"
#include "board.h"

#define DBG_TAG "MAIN"
#include "log.h"

int main(void)
{
    board_init();

    while (1) {
        LOG_F("hello why!!!");
        LOG_F("1hello world fatal\r\n");
        LOG_E("2hello world error\r\n");
        LOG_W("3hello world warning\r\n");
        LOG_I("4hello world information\r\n");
        LOG_D("5hello world debug\r\n");
        LOG_T("6hello world trace\r\n");
        LOG_RF("7hello world fatal raw\r\n");
        LOG_RE("8hello world error raw\r\n");
        LOG_RW("9hello world warning raw\r\n");
        LOG_RI("10hello world information raw\r\n");
        LOG_RD("11hello world debug raw\r\n");
        LOG_RT("12hello world trace raw\r\n");
        bflb_mtimer_delay_ms(1000);
    }
}

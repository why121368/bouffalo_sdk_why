
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <lwip/api.h>
#include <lwip/arch.h>
#include <lwip/opt.h>
#include <lwip/inet.h>
#include <lwip/errno.h>
#include <netdb.h>

#include "shell.h"
#include "utils_getopt.h"
#include "bflb_mtimer.h"

#include "bflb_adc.h"

#define IP "183.230.40.40"   //onenet server ip
#define PORT "1811"          //onenet server port
#define DEVICEID "606400"    //产品ID  
#define AUTHENTICATION "room001blf" //鉴权信息
#define LUANAME "test"  //脚本名称

// clang-format off
static const uint8_t send_buf[128]={ 0 };
uint32_t recv_buf[128] = { 0 };
// clang-format on

shell_sig_func_ptr abort_exec;
uint64_t total_cnt = 0;
int sock_client = -1;

struct bflb_device_s *adc;

static void test_close(int sig)
{
    if (sock_client) {
        closesocket(sock_client);
    }
    abort_exec(sig);
    if (total_cnt > 0) {
        printf("Total send data=%lld\r\n", total_cnt);
    }
}

#define PING_USAGE                    \
    "wifi_tcp_test [ip] [port]\r\n"   \
    "\t ip: dest ip or server ip\r\n" \
    "\t port: dest server listen port\r\n"

static void wifi_test_tcp_client_init(int argc, char **argv)
{
    float temp = 0.0;
    float average_filter = 0.0;

    //adc temp
    adc = bflb_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct bflb_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = false;
    cfg.continuous_conv_mode = false;
    cfg.differential_mode = false;
    cfg.resolution = ADC_RESOLUTION_16B;
    cfg.vref = ADC_VREF_2P0V;

    struct bflb_adc_channel_s chan;

    chan.pos_chan = ADC_CHANNEL_TSEN_P;
    chan.neg_chan = ADC_CHANNEL_GND;

    bflb_adc_init(adc, &cfg);
    bflb_adc_channel_config(adc, &chan, 1);
    bflb_adc_tsen_init(adc, ADC_TSEN_MOD_INTERNAL_DIODE);

    abort_exec = shell_signal(SHELL_SIGINT, test_close);
    printf("tcp client task start ...\r\n");

    char *addr;
    char *port;
    struct sockaddr_in remote_addr;

    if (argc > 1) {
        /* get address (argv[1] if present) */
        addr = argv[1];
    }
    else {
        addr = IP;
    }
    /* get port number (argv[2] if present) */
    if (argc > 2) {
        port = argv[2];
    } else {
        port = PORT;
    }

    while (1) {
        if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("TCP Client create socket error\r\n");
            return;
        }

        //准备数据
        for (int i = 0; i < 50; i++) {
            average_filter += bflb_adc_tsen_get_temp(adc);
            vTaskDelay(10);
        }
        temp=average_filter/50.0;
        average_filter = 0.0;

        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons(atoi(port));
        remote_addr.sin_addr.s_addr = inet_addr(addr);
        memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));

        printf("Server ip Address : %s:%s\r\n", addr, port);

        if (connect(sock_client, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) != 0) {
            printf("TCP client connect server falied!\r\n");
            closesocket(sock_client);
            return;
        }

        printf("TCP client connect server success!\r\n");
        printf("Press CTRL-C to exit.\r\n");
        total_cnt = 0;
        //发送登录报文
        sprintf(send_buf, "*%s#%s#%s*",DEVICEID,AUTHENTICATION,LUANAME);  // *产品ID#鉴权信息#脚本名称*
        printf("%s\r\n",send_buf);
        if(write(sock_client,send_buf,sizeof(send_buf))<0){
            printf("log in falied!\r\n");
        }
        
        sprintf(send_buf, "%.2f",temp);
        while (1) {
            if (write(sock_client, send_buf, strlen(send_buf)) < 0) {
                printf("write falied!\r\n");
                break;
            }
            total_cnt = recv(sock_client, (uint8_t *)recv_buf, sizeof(recv_buf), 0);
            if (total_cnt <= 0)
                break;
            printf("%s\r\n", (uint8_t *)recv_buf);
            vTaskDelay(3000);
        }
        closesocket(sock_client);
        return;
    }
}

#ifdef CONFIG_SHELL
#include <shell.h>

int cmd_wifi_tcp_client(int argc, char **argv)
{
    wifi_test_tcp_client_init(argc, argv);

    return 0;
}

SHELL_CMD_EXPORT_ALIAS(cmd_wifi_tcp_client, onenet_tcp, wifi tcp connect onenet);
#endif

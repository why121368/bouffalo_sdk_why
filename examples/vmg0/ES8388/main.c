/**
 * @file main.c
 * @brief
 *
 * Copyright (c) 2021 Bouffalolab team
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 *
 */
#include "board.h"
#include "bflb_gpio.h"
#include "bflb_l1c.h"
#include "bflb_mtimer.h"


#include "bflb_i2c.h"


#include "bl616_glb.h"
#include "bflb_dma.h"
#include "bsp_es8388.h"
#include "bflb_i2s.h"


// 8388
struct bflb_device_s *i2s0;
struct bflb_device_s *dma0_ch0;
struct bflb_device_s *dma0_ch1;

uint8_t rx_buffer[32000];

static ES8388_Cfg_Type ES8388Cfg = {
    .work_mode = ES8388_CODEC_MDOE,            /*!< ES8388 work mode */
    .role = ES8388_SLAVE,                      /*!< ES8388 role */
    .mic_input_mode = ES8388_SINGLE_ENDED_MIC, /*!< ES8388 mic input mode */
    .mic_pga = ES8388_MIC_PGA_6DB,             /*!< ES8388 mic PGA */
    .i2s_frame = ES8388_LEFT_JUSTIFY_FRAME,    /*!< ES8388 I2S frame */
    .data_width = ES8388_DATA_LEN_16,          /*!< ES8388 I2S dataWitdh */
};

void dma0_ch0_isr(void *arg)
{
    printf("tc done\r\n");
}

void dma0_ch1_isr(void *arg)
{
    printf("rx done\r\n");
}

void i2s_gpio_init()
{
    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");

    /* I2S_FS */
    bflb_gpio_init(gpio, GPIO_PIN_1, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_DI */
    bflb_gpio_init(gpio, GPIO_PIN_10, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_DO */
    bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* I2S_BCLK */
    bflb_gpio_init(gpio, GPIO_PIN_0, GPIO_FUNC_I2S | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
    /* MCLK CLKOUT */
    bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_FUNC_CLKOUT | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
}

void i2s_dma_init()
{
    static struct bflb_dma_channel_lli_pool_s tx_llipool[100];
    static struct bflb_dma_channel_lli_transfer_s tx_transfers[1];
    static struct bflb_dma_channel_lli_pool_s rx_llipool[100];
    static struct bflb_dma_channel_lli_transfer_s rx_transfers[1];

    struct bflb_i2s_config_s i2s_cfg = {
        .bclk_freq_hz = 16000 * 16 * 2, /* bclk = Sampling_rate * frame_width * channel_num */
        .role = I2S_ROLE_MASTER,
        .format_mode = I2S_MODE_LEFT_JUSTIFIED,
        .channel_mode = I2S_CHANNEL_MODE_NUM_2,
        .frame_width = I2S_SLOT_WIDTH_16,
        .data_width = I2S_SLOT_WIDTH_16,
        .fs_offset_cycle = 0,

        .tx_fifo_threshold = 0,
        .rx_fifo_threshold = 0,
    };

    struct bflb_dma_channel_config_s tx_config = {
        .direction = DMA_MEMORY_TO_PERIPH,
        .src_req = DMA_REQUEST_NONE,
        .dst_req = DMA_REQUEST_I2S_TX,
        .src_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT,
    };

    struct bflb_dma_channel_config_s rx_config = {
        .direction = DMA_PERIPH_TO_MEMORY,
        .src_req = DMA_REQUEST_I2S_RX,
        .dst_req = DMA_REQUEST_NONE,
        .src_addr_inc = DMA_ADDR_INCREMENT_DISABLE,
        .dst_addr_inc = DMA_ADDR_INCREMENT_ENABLE,
        .src_burst_count = DMA_BURST_INCR1,
        .dst_burst_count = DMA_BURST_INCR1,
        .src_width = DMA_DATA_WIDTH_16BIT,
        .dst_width = DMA_DATA_WIDTH_16BIT
    };
    printf("i2s init\r\n");
    i2s0 = bflb_device_get_by_name("i2s0");
    /* i2s init */
    bflb_i2s_init(i2s0, &i2s_cfg);
    /* enable dma */
    bflb_i2s_link_txdma(i2s0, true);
    bflb_i2s_link_rxdma(i2s0, true);

    printf("dma init\r\n");
    dma0_ch0 = bflb_device_get_by_name("dma0_ch0");
    dma0_ch1 = bflb_device_get_by_name("dma0_ch1");

    bflb_dma_channel_init(dma0_ch0, &tx_config);
    bflb_dma_channel_init(dma0_ch1, &rx_config);

    bflb_dma_channel_irq_attach(dma0_ch0, dma0_ch0_isr, NULL);
    bflb_dma_channel_irq_attach(dma0_ch1, dma0_ch1_isr, NULL);

    tx_transfers[0].src_addr = (uint32_t)rx_buffer;
    tx_transfers[0].dst_addr = (uint32_t)DMA_ADDR_I2S_TDR;
    tx_transfers[0].nbytes = sizeof(rx_buffer);

    rx_transfers[0].src_addr = (uint32_t)DMA_ADDR_I2S_RDR;
    rx_transfers[0].dst_addr = (uint32_t)rx_buffer;
    rx_transfers[0].nbytes = sizeof(rx_buffer);

    printf("dma lli init\r\n");
    uint32_t num = bflb_dma_channel_lli_reload(dma0_ch0, tx_llipool, 100, tx_transfers, 1);
    bflb_dma_channel_lli_link_head(dma0_ch0, tx_llipool, num);
    printf("tx dma lli num: %d \r\n", num);
    num = bflb_dma_channel_lli_reload(dma0_ch1, rx_llipool, 100, rx_transfers, 1);
    printf("rx dma lli num: %d \r\n", num);
    bflb_dma_channel_lli_link_head(dma0_ch1, rx_llipool, num);
    bflb_dma_channel_start(dma0_ch0);
    bflb_dma_channel_start(dma0_ch1);
}

void mclk_out_init()
{
    /* output MCLK,
    Will change the clock source of i2s,
    It needs to be called before i2s is initialized
    clock source 25M
    */
    GLB_Set_I2S_CLK(ENABLE, 2, GLB_I2S_DI_SEL_I2S_DI_INPUT, GLB_I2S_DO_SEL_I2S_DO_OUTPT);
    GLB_Set_Chip_Clock_Out2_Sel(GLB_CHIP_CLK_OUT_2_I2S_REF_CLK);
}


int main(void)
{
    board_init();
    board_i2c0_gpio_init();

    printf("lvgl case\r\n");

        /* init ES8388 Codec */
        printf("es8388 init\n\r");
        ES8388_Init(&ES8388Cfg);
        ES8388_Set_Voice_Volume(90);

        /* gpio init */
        i2s_gpio_init();
        /* mclk clkout init */
        mclk_out_init();

        /* i2s init */
        i2s_dma_init();

        /* enable i2s tx and rx */
        bflb_i2s_feature_control(i2s0, I2S_CMD_DATA_ENABLE, I2S_CMD_DATA_ENABLE_TX | I2S_CMD_DATA_ENABLE_RX);
   

    while (1) {
        bflb_mtimer_delay_ms(1);
    }
}
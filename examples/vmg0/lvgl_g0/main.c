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

#include "lv_conf.h"
#include "lvgl.h"

#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "lcd.h"

#include "bflb_i2c.h"
#include "bflb_cam.h"
#include "image_sensor.h"

#include "bl616_glb.h"
#include "bflb_dma.h"
#include "bflb_i2s.h"


#define CROP_WQVGA_X        (240)
#define CROP_WQVGA_Y        (320)
#define CAM_BUFF_NUM        (4)



/* lvgl log cb */
void lv_log_print_g_cb(const char *buf)
{
    printf("[LVGL] %s", buf);
}

int main(void)
{
    // cam
    uint8_t *pic;
    static uint8_t picture[CROP_WQVGA_X * CROP_WQVGA_Y * CAM_BUFF_NUM] ATTR_NOINIT_PSRAM_SECTION __attribute__((aligned(64)));

    lv_img_dsc_t img_src_cam = {
        .header.always_zero = 0,
        .header.w = CROP_WQVGA_X,
        .header.h = CROP_WQVGA_Y,
        .data_size = CROP_WQVGA_X * CROP_WQVGA_Y * (CAM_BUFF_NUM / 2),
        .header.cf = LV_IMG_CF_TRUE_COLOR,
    };

    board_init();
    printf("lvgl case\r\n");
    
    /* lvgl init */
    lv_log_register_print_cb(lv_log_print_g_cb);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
#if LV_USE_DEMO_BENCHMARK
    lv_demo_benchmark();
#endif

#if LV_USE_DEMO_WIDGETS
    // lv_demo_widgets();
    { // 图片显示
        lv_obj_t *scr = lv_scr_act();
        LV_IMG_DECLARE(img_demo_widgets_avatar);
        lv_obj_t *avatar = lv_img_create(scr);
        lv_img_set_src(avatar, &img_demo_widgets_avatar);
    }
#endif

    // Cam test
    static struct bflb_device_s *i2c0;
    static struct bflb_device_s *cam0;

    // static struct bflb_device_s *uart0;
    // uart0 = bflb_device_get_by_name("uart0");

    uint32_t i, j, pic_size;
    struct bflb_cam_config_s cam_config;
    struct image_sensor_config_s *sensor_config;
    board_dvp_gpio_init();
    board_i2c0_gpio_init();

    i2c0 = bflb_device_get_by_name("i2c0");
    cam0 = bflb_device_get_by_name("cam0");

    if (image_sensor_scan(i2c0, &sensor_config)) {
        printf("\r\nSensor name: %s\r\n", sensor_config->name);
    } else {
        printf("\r\nError! Can't identify sensor!\r\n");
        while (1) {
        }
    }

    /* Crop resolution_x, should be set before init */
    bflb_cam_crop_hsync(cam0, 112, 112 + CROP_WQVGA_X);
    /* Crop resolution_y, should be set before init */
    bflb_cam_crop_vsync(cam0, 120, 120 + CROP_WQVGA_Y);

    memcpy(&cam_config, sensor_config, IMAGE_SENSOR_INFO_COPY_SIZE);
    cam_config.with_mjpeg = false;
    cam_config.input_source = CAM_INPUT_SOURCE_DVP;
    cam_config.output_format = CAM_OUTPUT_FORMAT_AUTO;
    cam_config.output_bufaddr = (uint32_t)picture;
    cam_config.output_bufsize = CROP_WQVGA_X * CROP_WQVGA_Y * (CAM_BUFF_NUM / 2);

    bflb_cam_init(cam0, &cam_config);
    bflb_cam_start(cam0);

    // bflb_cam_stop(cam0);

    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *avatar = lv_img_create(scr);
    lv_img_set_angle(avatar, 0);
    lv_obj_align(avatar, LV_ALIGN_CENTER, -1, -1);
    lv_img_set_src(avatar, &img_src_cam);
    // !Cam test

    lv_task_handler();

    printf("lvgl success\r\n");

    j = 0;
    while (1) {
        lv_task_handler();
        bflb_mtimer_delay_ms(1);

        // CAM test
        if (bflb_cam_get_frame_count(cam0) > 0) {
            // bflb_cam_stop(cam0);
            pic_size = bflb_cam_get_frame_info(cam0, &pic);
            bflb_cam_pop_one_frame(cam0);
            printf("pop picture %d: 0x%08x, len: %d\r\n", j, (uint32_t)pic, pic_size);
            j ++;
            // for(uint32_t c = 0; c < pic_size; c ++)  //串口发送数据到上位机观察图像
            // {
            //     bflb_uart_putchar(uart0, pic[c]);
            // }
            
            img_src_cam.data = pic;
            lv_img_set_src(avatar, &img_src_cam);
        }
    }
}

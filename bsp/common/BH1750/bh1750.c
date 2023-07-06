/**
 * @file bh1750.c
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
 */

#include "bflb_i2c.h"
#include "bflb_mtimer.h"
#include "bh1750.h"


static uint8_t   Buffer[8];       // 接收数据缓存区

static struct bflb_device_i2c *i2c0;
static struct bflb_i2c_msg_s msgs[1];

//写命令
static void BH1750_WriteOrder(uint8_t REG_Address)
{
    msgs[0].addr = BH1750_ADDR;             
    msgs[0].flags = I2C_M_NOSTART;
    msgs[0].buffer = &REG_Address;
    msgs[0].length = 1;

    bflb_i2c_transfer(i2c0, msgs, 1);
}


//初始化BH1750，根据需要请参考pdf进行修改
void BH1750_Init()
{
    i2c0 = bflb_device_get_by_name("i2c0");
    bflb_i2c_init(i2c0,200000);

    BH1750_WriteOrder(BH1750_POWER_DOWN);
    bflb_mtimer_delay_ms(180);                // 延时180ms
}


//连续读出BH1750内部数据
static void BH1750_ReadData(void)
{
    msgs[0].addr = BH1750_ADDR;             
    msgs[0].flags = I2C_M_READ;
    msgs[0].buffer = Buffer;
    msgs[0].length = 2;
    bflb_i2c_transfer(i2c0, msgs, 1);
    bflb_mtimer_delay_ms(5);
}

float BH1750_GetData(void)
{
    uint32_t dataTem;                      // 结果值
    static uint8_t flagInit = 0;
    if(flagInit==0)
    {
        BH1750_WriteOrder(BH1750_POWER_ON);           // power on
        BH1750_WriteOrder(BH1750_CONTINUOUS_HIGH_RES_MODE);           // H- resolution mode
        bflb_mtimer_delay_ms(180);                     // 延时180ms
        flagInit=1;
    }        
    BH1750_ReadData();                     // 读取原始数据，存储在Buffer中
    dataTem = Buffer[0];
    dataTem = (dataTem << 8) + Buffer[1];  // 转换成结果光强度值
    return dataTem / 1.2;
}

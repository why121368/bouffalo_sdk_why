/**
 * @file bh1750.h
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

#ifndef __BH1750_H__
#define __BH1750_H__

// IIC器件地址
#define    BH1750_ADDR   0x23               // 定义器件在IIC总线中的从地址, 根据ADDR引脚不同修改：当ADDR引脚接GND或空置时地址为0x23，接3.3V时地址为0x5C
#define    BH1750_POWER_DOWN   0x00
#define    BH1750_POWER_ON     0x01
#define    BH1750_RESET        0x07
#define    BH1750_CONTINUOUS_HIGH_RES_MODE    0x10
#define    BH1750_CONTINUOUS_HIGH_RES_MODE2   0x11
#define    BH1750_CONTINUOUS_LOW_RES_MODE     0x13
#define    BH1750_ONE_TIME_HIGH_RES_MODE      0x20
#define    BH1750_ONE_TIME_HIGH_RES_MODE2     0x21
#define    BH1750_ONE_TIME_LOW_RES_MODE       0x23



/*****************************************************************************
 ** 声明  全局函数
****************************************************************************/
void     BH1750_Init(void);           // 初始化BH1750
float    BH1750_GetData(void);        // 获取光照大小



#endif /* __BH1750_H__ */
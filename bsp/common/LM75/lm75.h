/*
    For more information, visit https://github.com/why121368/bouffalo_sdk_why.
    Created by Henry Wang, at Harbin Institute of Technology, on July 6, 2023.
*/

#ifndef __LM75_H__
#define __LM75_H__


#define		LM75_ADDR		        0X48    // LM75的I2C从地址
#define		LM75_REG_TEMP	    	0X00    // 保存温度数据的寄存器地址
#define		LM75_REG_CONF		    0X01    // 保存配置信息的寄存器地址
#define		LM75_REG_THYST		    0X02    // 保存迟滞温度值的寄存器地址
#define		LM75_REG_TOS			0X03    // 保存报警温度的寄存器地址

/*****************************************************************************
 ** 声明  全局函数
****************************************************************************/

void     LM75_Init(void);           // 初始化BH1750
float    LM75_GetData(void);        // 获取温度数值



#endif /* __LM75_H__ */
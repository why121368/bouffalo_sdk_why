/*
    For more information, visit https://github.com/why121368/bouffalo_sdk_why.
    Created by Henry Wang, at Harbin Institute of Technology, on July 6, 2023.
*/

#include "bflb_i2c.h"
#include "bflb_mtimer.h"
#include "lm75.h"


static uint8_t   Buffer[8];       // �������ݻ�����

static struct bflb_device_i2c *i2c0;
static struct bflb_i2c_msg_s msgs[1];

//д����
static void LM75_WriteOrder(uint8_t REG_Address)
{
    msgs[0].addr = LM75_ADDR;             
    msgs[0].flags = I2C_M_NOSTART;
    msgs[0].buffer = &REG_Address;
    msgs[0].length = 1;

    bflb_i2c_transfer(i2c0, msgs, 1);
}


//��ʼ��LM75��������Ҫ��ο�pdf�����޸�
void LM75_Init()
{
    i2c0 = bflb_device_get_by_name("i2c0");
    bflb_i2c_init(i2c0,200000);

    LM75_WriteOrder(LM75_POWER_DOWN);
    bflb_mtimer_delay_ms(180);                // ��ʱ180ms
}


//��������LM75�ڲ�����
static void LM75_ReadData(void)
{
    msgs[0].addr = LM75_ADDR;             
    msgs[0].flags = I2C_M_READ;
    msgs[0].buffer = Buffer;
    msgs[0].length = 2;
    bflb_i2c_transfer(i2c0, msgs, 1);
    bflb_mtimer_delay_ms(5);
}

float LM75_GetData(void)
{
    uint32_t dataTem;                      // ���ֵ
    static uint8_t flagInit = 0;
    if(flagInit==0)
    {
        LM75_WriteOrder(LM75_POWER_ON);           // power on
        LM75_WriteOrder(LM75_CONTINUOUS_HIGH_RES_MODE);           // H- resolution mode
        bflb_mtimer_delay_ms(180);                     // ��ʱ180ms
        flagInit=1;
    }        
    LM75_ReadData();                     // ��ȡԭʼ���ݣ��洢��Buffer��
    dataTem = Buffer[0];
    dataTem = (dataTem << 8) + Buffer[1];  // ת���ɽ����ǿ��ֵ
    return dataTem / 1.2;
}

/*
 * [Warning!] This file is auto-generated by pika compiler.
 * Do not edit it manually.
 * The source code is *.pyi file.
 * More details: 
 * English Doc:
 * https://pikadoc.readthedocs.io/en/latest/PikaScript%20%E6%A8%A1%E5%9D%97%E6%A6%82%E8%BF%B0.html
 * Chinese Doc:
 * https://pikadoc.readthedocs.io/zh/latest/PikaScript%20%E6%A8%A1%E5%9D%97%E6%A6%82%E8%BF%B0.html
 */

#ifndef ___time__H
#define ___time__H
#include <stdio.h>
#include <stdlib.h>
#include "PikaObj.h"

PikaObj *New__time(Args *args);

void _time___init__(PikaObj *self);
void _time_asctime(PikaObj *self);
void _time_ctime(PikaObj *self, pika_float unix_time);
void _time_gmtime(PikaObj *self, pika_float unix_time);
void _time_localtime(PikaObj *self, pika_float unix_time);
int _time_mktime(PikaObj *self);
void _time_platformGetTick(PikaObj *self);
void _time_sleep_ms(PikaObj *self, int ms);
void _time_sleep_s(PikaObj *self, int s);
pika_float _time_time(PikaObj *self);
int _time_time_ns(PikaObj *self);

#endif

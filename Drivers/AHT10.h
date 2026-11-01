#ifndef __AHT10_H
#define __AHT10_H

#include "i2c.h"

//读取AHT10数据
int aht10_read(float* humi,float* temp);

#endif

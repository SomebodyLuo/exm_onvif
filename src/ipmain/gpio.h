#ifndef __GPIO_H__
#define __GPIO_H__

typedef struct TimeDelay
{
        unsigned char beginToEnd[8];         //0-1����ʱ
        unsigned char endToBegin[8];         //1-0����ʱ
} __attribute__((packed))TimeDelay;

#endif

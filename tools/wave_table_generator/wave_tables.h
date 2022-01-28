#ifndef __WAVE_TABLES_H__
#define __WAVE_TABLES_H__

#include <stdlib.h>
#include <stdint.h>

#define WAVE_TABLE_SIZE 2048

extern const int16_t sine_table[];
extern const int16_t triang_table[];
extern const int16_t saw1_table[];
extern const int16_t saw2_table[];
extern const int16_t square_table[];

#endif
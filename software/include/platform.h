#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"

void platform_init(void);
void platform_swap_buffers(void);
void put_pixel(int x, int y, short int color);

int platform_read_keys(void);
int platform_read_switches(void);
void platform_clear_screen(void);
void wait_for_vsync(void);
void printuart(const char *ptr);

#endif
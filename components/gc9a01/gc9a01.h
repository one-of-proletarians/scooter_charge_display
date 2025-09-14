//--------------------------------------------------------------------------------------------------------
// Nadyrshin Ruslan - [YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ]
// Liyanboy74
//--------------------------------------------------------------------------------------------------------
#ifndef _GC9A01_H
#define _GC9A01_H

#define GC9A01_Width CONFIG_GC9A01_Width
#define GC9A01_Height CONFIG_GC9A01_Height

#include "lvgl.h"

void gc9a01_init();
void gc9a01_draw(const lv_area_t *area, uint8_t *px_map);
#endif

#pragma once
#include <pebble.h>

// [ ]Aplite, [*]Basalt, [*]Chalk, [ ]Diorite, [*]Emery, [ ]Flint, [*]Gabbro

#define PADDING PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 9, 28, 0, 15, 0, 45)
#define DOT_RADIUS PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 5, 5, 0, 7, 0, 7)
#define DOT_GAP PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 9, 9, 0, 12, 0, 12)
#define DOT_COUNT 7

typedef enum {
  PERSIST_KEY_WEATHER_BASE = 100,
  PERSIST_KEY_TEMP         = 110,
  PERSIST_KEY_CONDITIONS   = 111,
  PERSIST_KEY_STEP_GOAL    = 120,
  PERSIST_KEY_SUNRISE      = 130,
  PERSIST_KEY_SUNSET       = 131,
} PersistKey;

void draw_standard_dots(GContext *ctx, int y, GColor *colors, int outline_index);
int get_current_wday(void);
void set_current_wday(int wday);

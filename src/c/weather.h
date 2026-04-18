#pragma once
#include "dots_common.h"

void weather_init(Layer *weather_layer);
void weather_inbox_received(DictionaryIterator *iterator);
void draw_weather_dots(Layer *layer, GContext *ctx);

#pragma once
#include "dots_common.h"

void sun_init(Layer *sun_layer);
void sun_inbox_received(DictionaryIterator *iterator);
void draw_sun_dots(Layer *layer, GContext *ctx);

#pragma once
#include "dots_common.h"

void battery_init(Layer *battery_layer);
void battery_callback(BatteryChargeState state);
void draw_battery_dots(Layer *layer, GContext *ctx);

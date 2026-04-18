#include "battery_dots.h"

static int s_battery_level;
static Layer *s_battery_layer_ref;

void battery_init(Layer *battery_layer) {
  s_battery_layer_ref = battery_layer;
  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());
}

void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer_ref);
}

void draw_battery_dots(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int y = bounds.size.h / 2;

  int filled = (s_battery_level > 0) ? (s_battery_level + 14) / 15 : 0;
  if (filled > DOT_COUNT) filled = DOT_COUNT;

  GColor colors[DOT_COUNT];
  int outline_index = -1;

  for (int i = 0; i < DOT_COUNT; i++) {
    bool is_filled = (i >= DOT_COUNT - filled);
    colors[i] = is_filled ? GColorKellyGreen : GColorLightGray;

    if (is_filled && i == DOT_COUNT - filled) {
      outline_index = i;
    }
  }

  draw_standard_dots(ctx, y, colors, outline_index);
}

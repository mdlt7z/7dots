#include "day_dots.h"

void draw_day_dots(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int y = bounds.size.h / 2;

  int today = get_current_wday();
  GColor colors[DOT_COUNT];

  for (int i = 0; i < DOT_COUNT; i++) {
    if (i == 0) {
      colors[i] = GColorSunsetOrange;
    } else if (i == 6) {
      colors[i] = GColorPictonBlue;
    } else {
      colors[i] = GColorLightGray;
    }
  }

  draw_standard_dots(ctx, y, colors, today);
}

#include "dots_common.h"

static int s_current_wday = 0;

int get_current_wday(void) {
  return s_current_wday;
}

void set_current_wday(int wday) {
  s_current_wday = wday;
}

void draw_standard_dots(GContext *ctx, int y, GColor *colors, int outline_index) {
  for (int i = 0; i < DOT_COUNT; i++) {
    int x = PADDING + DOT_RADIUS + i * (DOT_GAP + DOT_RADIUS * 2);

    graphics_context_set_fill_color(ctx, colors[i]);
    graphics_fill_circle(ctx, GPoint(x, y), DOT_RADIUS);

    if (i == outline_index) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_circle(ctx, GPoint(x, y), DOT_RADIUS);
    }
  }
}

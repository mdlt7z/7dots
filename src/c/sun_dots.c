#include "sun_dots.h"

static Layer *s_sun_layer_ref;
static time_t s_sunrise = 0;
static time_t s_sunset = 0;

void sun_init(Layer *sun_layer) {
  s_sun_layer_ref = sun_layer;

  if (persist_exists(PERSIST_KEY_SUNRISE)) {
    s_sunrise = (time_t)persist_read_int(PERSIST_KEY_SUNRISE);
  }
  if (persist_exists(PERSIST_KEY_SUNSET)) {
    s_sunset = (time_t)persist_read_int(PERSIST_KEY_SUNSET);
  }
}

void sun_inbox_received(DictionaryIterator *iterator) {
  Tuple *sr = dict_find(iterator, MESSAGE_KEY_SUNRISE);
  Tuple *ss = dict_find(iterator, MESSAGE_KEY_SUNSET);

  if (sr) {
    s_sunrise = (time_t)sr->value->int32;
    persist_write_int(PERSIST_KEY_SUNRISE, (int)s_sunrise);
  }
  if (ss) {
    s_sunset = (time_t)ss->value->int32;
    persist_write_int(PERSIST_KEY_SUNSET, (int)s_sunset);
  }
  layer_mark_dirty(s_sun_layer_ref);
}

void draw_sun_dots(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int y = bounds.size.h / 2;

  if (s_sunrise == 0 || s_sunset == 0) {
    GColor colors[DOT_COUNT];
    for (int i = 0; i < DOT_COUNT; i++) {
      colors[i] = GColorLightGray;
    }
    draw_standard_dots(ctx, y, colors, -1);
    return;
  }

  time_t now = time(NULL);
  bool is_daytime = (now >= s_sunrise && now < s_sunset);

  int progress_dot;
  if (is_daytime) {
    int total = (int)(s_sunset - s_sunrise);
    int elapsed = (int)(now - s_sunrise);
    if (total <= 0) total = 1;
    progress_dot = (elapsed * 6) / total;
  } else {
    time_t night_start = s_sunset;
    time_t night_end = s_sunrise + 86400;
    if (now < s_sunrise) {
      night_start = s_sunset - 86400;
      night_end = s_sunrise;
    }
    int total = (int)(night_end - night_start);
    int elapsed = (int)(now - night_start);
    if (total <= 0) total = 1;
    progress_dot = (elapsed * 6) / total;
  }
  if (progress_dot < 0) progress_dot = 0;
  if (progress_dot > 6) progress_dot = 6;


  GColor left_color, right_color, middle_color;
  if (is_daytime) {
    left_color = GColorBrilliantRose;
    right_color = GColorSunsetOrange;
    middle_color = GColorPictonBlue;
  } else {
    left_color = GColorSunsetOrange;
    right_color = GColorBrilliantRose;
    middle_color = GColorCobaltBlue;
  }

  GColor colors[DOT_COUNT];
  for (int i = 0; i < DOT_COUNT; i++) {
    if (i == progress_dot) {
      colors[i] = GColorYellow;
    } else if (i == 0) {
      colors[i] = left_color;
    } else if (i == 6) {
      colors[i] = right_color;
    } else {
      colors[i] = middle_color;
    }
  }

  draw_standard_dots(ctx, y, colors, progress_dot);
}

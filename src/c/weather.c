#include "weather.h"

static int s_weather_codes[7] = {-1, -1, -1, -1, -1, -1, -1};
static Layer *s_weather_layer_ref;

static GColor weather_code_to_color(int code) {
  if (code < 0)  return GColorLightGray;        // データなし
  if (code == 1) return GColorSunsetOrange;     // 快晴
  if (code == 2) return GColorChromeYellow;     // 晴れ
  if (code == 3) return GColorLightGray;        // 曇り
  if (code <= 48) return GColorDarkGray;        // 霧
  if (code <= 57) return GColorPictonBlue;      // 小雨・霧雨
  if (code <= 67) return GColorPictonBlue;      // 雨
  if (code <= 77) return GColorTiffanyBlue;     // 雪
  if (code <= 82) return GColorPictonBlue;      // にわか雨
  if (code <= 86) return GColorTiffanyBlue;     // にわか雪
  if (code <= 99) return GColorLiberty;         // 雷雨
  return GColorLightGray;
}

void weather_init(Layer *weather_layer) {
  s_weather_layer_ref = weather_layer;

  for (int i = 0; i < 7; i++) {
    if (persist_exists(PERSIST_KEY_WEATHER_BASE + i)) {
      s_weather_codes[i] = persist_read_int(PERSIST_KEY_WEATHER_BASE + i);
    }
  }
}

void weather_inbox_received(DictionaryIterator *iterator) {
  for (int i = 0; i < 7; i++) {
    Tuple *t = dict_find(iterator, MESSAGE_KEY_WEATHER_DAY_0 + i);

    int today_wday = get_current_wday();

    if (t) {
      int index = (i + today_wday) % 7;
      s_weather_codes[index] = (int)t->value->int32;
      persist_write_int(PERSIST_KEY_WEATHER_BASE + index, s_weather_codes[index]);
    }
  }

  layer_mark_dirty(s_weather_layer_ref);
}

void draw_weather_dots(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int y = bounds.size.h / 2;

  int today_wday = get_current_wday();
  GColor colors[DOT_COUNT];

  for (int i = 0; i < DOT_COUNT; i++) {
    colors[i] = weather_code_to_color(s_weather_codes[i]);
  }

  draw_standard_dots(ctx, y, colors, today_wday);
}

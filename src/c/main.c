#include <pebble.h>
#include "dots_common.h"
#include "day_dots.h"
#include "steps_dots.h"
#include "battery_dots.h"
#include "weather.h"
#include "sun_dots.h"

static Window *s_main_window;
static Layer *s_window_layer;
static Layer *s_day_layer;
static Layer *s_steps_layer;
static Layer *s_battery_layer;
static Layer *s_weather_layer;
static Layer *s_sun_layer;
static TextLayer *s_weather_text;
static TextLayer *s_time_layer;

static char s_weather_text_buffer[42];
static char s_temperature_buffer[8];
static char s_conditions_buffer[32];
static char s_time_buffer[8];

// —— Time ——

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  set_current_wday(tick_time->tm_wday);

  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);

  text_layer_set_text(s_time_layer, s_time_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // refresh
  if (units_changed & DAY_UNIT) {
    if (s_day_layer) layer_mark_dirty(s_day_layer);
    if (s_steps_layer) layer_mark_dirty(s_steps_layer);
    if (s_weather_layer) layer_mark_dirty(s_weather_layer);
    if (s_sun_layer) layer_mark_dirty(s_sun_layer);
  }

  // refresh whether
  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
    app_message_outbox_send();
  }
}

// —— AppMessage ——

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  Tuple *step_goal_t = dict_find(iterator, MESSAGE_KEY_StepGoal);

  if (temp_tuple && conditions_tuple) {
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%d°C", (int)temp_tuple->value->int32);
    snprintf(s_conditions_buffer, sizeof(s_conditions_buffer), "%s", conditions_tuple->value->cstring);
    snprintf(s_weather_text_buffer, sizeof(s_weather_text_buffer), "%s %s", s_temperature_buffer, s_conditions_buffer);
    text_layer_set_text(s_weather_text, s_weather_text_buffer);

    persist_write_int(PERSIST_KEY_TEMP, (int)temp_tuple->value->int32);
    persist_write_string(PERSIST_KEY_CONDITIONS, conditions_tuple->value->cstring);
  }

  if (step_goal_t) {
    int goal = (int)step_goal_t->value->int32;
    if (goal > 0) {
      steps_set_goal(goal);
      persist_write_int(PERSIST_KEY_STEP_GOAL, goal);
      layer_mark_dirty(s_steps_layer);
    }
  }

  weather_inbox_received(iterator);
  sun_inbox_received(iterator);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// —— Window ——

// [ ]Aplite, [*]Basalt, [*]Chalk, [ ]Diorite, [*]Emery, [ ]Flint, [*]Gabbro

#define FONT_SMALL FONT_KEY_GOTHIC_18_BOLD
#define FONT_RECT FONT_KEY_LECO_36_BOLD_NUMBERS
#define FONT_ROUND FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM

#define FONT_QUICK_TIME PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, FONT_SMALL, FONT_KEY_GOTHIC_14_BOLD, FONT_SMALL, FONT_SMALL, FONT_KEY_LECO_20_BOLD_NUMBERS, FONT_SMALL, FONT_SMALL)
#define FONT_QUICK_WEATHER PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, FONT_SMALL, FONT_KEY_GOTHIC_14_BOLD, FONT_SMALL, FONT_SMALL, FONT_SMALL, FONT_SMALL, FONT_SMALL)
#define QUICK_Y PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 19, 0, 0, 28, 0, 0)

#define TIME_Y PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 46, 42, 0, 52, 0, 54)
#define WEATHER_Y PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 64, 16, 0, 70, 0, 28)
#define ROUND_TIME_Y PBL_PLATFORM_SWITCH(PBL_PLATFORM_TYPE_CURRENT, 0, 0, 18, 0, 0, 0, 24)

static void prv_unobstructed_will_change(GRect now_bounds, void *context) {
  GRect full_bounds = layer_get_bounds(s_window_layer);
  bool is_hide = !grect_equal(&full_bounds, &now_bounds);

  if (is_hide) {
    text_layer_set_text_alignment(s_weather_text, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
    text_layer_set_font(s_weather_text, fonts_get_system_font(FONT_QUICK_WEATHER));
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_QUICK_TIME));
  } else {
    text_layer_set_text_alignment(s_weather_text, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
    text_layer_set_font(s_weather_text, fonts_get_system_font(FONT_SMALL));
    text_layer_set_font(s_time_layer, fonts_get_system_font(PBL_IF_ROUND_ELSE(FONT_ROUND, FONT_RECT)));
  }
}

static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);
  GRect full_bounds = layer_get_bounds(s_window_layer);
  bool is_hide = !grect_equal(&full_bounds, &bounds);

  GRect time_frame = layer_get_frame(text_layer_get_layer(s_time_layer));
  time_frame.origin.y = is_hide ? (bounds.size.h - QUICK_Y) : (full_bounds.size.h - TIME_Y);
  layer_set_frame(text_layer_get_layer(s_time_layer), time_frame);

  GRect weather_frame = layer_get_frame(text_layer_get_layer(s_weather_text));
  weather_frame.origin.y = is_hide ? (bounds.size.h - QUICK_Y) : (full_bounds.size.h - WEATHER_Y);
  layer_set_frame(text_layer_get_layer(s_weather_text), weather_frame);
}

static void main_window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(s_window_layer);

  int dot_height = DOT_RADIUS * 2 + PADDING * 2;
  int dot_y = DOT_GAP + DOT_RADIUS * 2;

  // day dots
  s_day_layer = layer_create(GRect(0, 0 + ROUND_TIME_Y, bounds.size.w, dot_height));
  layer_set_update_proc(s_day_layer, draw_day_dots);
  layer_add_child(s_window_layer, s_day_layer);

  // steps dots
  s_steps_layer = layer_create(GRect(0, dot_y + ROUND_TIME_Y, bounds.size.w, dot_height));
  layer_set_update_proc(s_steps_layer, draw_steps_dots);
  layer_add_child(s_window_layer, s_steps_layer);

  // weather dots
  s_weather_layer = layer_create(GRect(0, dot_y * 2 + ROUND_TIME_Y, bounds.size.w, dot_height));
  layer_set_update_proc(s_weather_layer, draw_weather_dots);
  layer_add_child(s_window_layer, s_weather_layer);

  // sun dots
  s_sun_layer = layer_create(GRect(0, dot_y * 3 + ROUND_TIME_Y, bounds.size.w, dot_height));
  layer_set_update_proc(s_sun_layer, draw_sun_dots);
  layer_add_child(s_window_layer, s_sun_layer);

  // battery dots
  s_battery_layer = layer_create(GRect(0, dot_y * 4 + ROUND_TIME_Y, bounds.size.w, dot_height));
  layer_set_update_proc(s_battery_layer, draw_battery_dots);
  layer_add_child(s_window_layer, s_battery_layer);

  // weather
  s_weather_text = text_layer_create(GRect(PADDING, PBL_IF_ROUND_ELSE(WEATHER_Y, bounds.size.h - WEATHER_Y), PBL_IF_ROUND_ELSE(bounds.size.w, bounds.size.w - PADDING * 2), 24));
  text_layer_set_background_color(s_weather_text, GColorClear);
  text_layer_set_text_color(s_weather_text, GColorBlack);
  text_layer_set_font(s_weather_text, fonts_get_system_font(FONT_SMALL));
  text_layer_set_text_alignment(s_weather_text, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  text_layer_set_text(s_weather_text, "Loading...");
  layer_add_child(s_window_layer, text_layer_get_layer(s_weather_text));

  if (persist_exists(PERSIST_KEY_TEMP) && persist_exists(PERSIST_KEY_CONDITIONS)) {
    int temp = persist_read_int(PERSIST_KEY_TEMP);
    persist_read_string(PERSIST_KEY_CONDITIONS, s_conditions_buffer, sizeof(s_conditions_buffer));
    snprintf(s_temperature_buffer, sizeof(s_temperature_buffer), "%d°C", temp);
    snprintf(s_weather_text_buffer, sizeof(s_weather_text_buffer), "%s %s", s_temperature_buffer, s_conditions_buffer);
    text_layer_set_text(s_weather_text, s_weather_text_buffer);
  }

  // time
  s_time_layer = text_layer_create(GRect(0, bounds.size.h - TIME_Y, PBL_IF_ROUND_ELSE(bounds.size.w, bounds.size.w - PADDING), 38));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(PBL_IF_ROUND_ELSE(FONT_ROUND, FONT_RECT)));
  text_layer_set_text_alignment(s_time_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentRight));
  layer_add_child(s_window_layer, text_layer_get_layer(s_time_layer));

  // init
  steps_init(s_steps_layer);
  weather_init(s_weather_layer);
  sun_init(s_sun_layer);
  battery_init(s_battery_layer);

  if (persist_exists(PERSIST_KEY_STEP_GOAL)) {
    steps_set_goal(persist_read_int(PERSIST_KEY_STEP_GOAL));
  }
  UnobstructedAreaHandlers handlers = {
    .will_change = prv_unobstructed_will_change,
    .change = prv_unobstructed_change,
  };
  unobstructed_area_service_subscribe(handlers, NULL);

  GRect now_bounds = layer_get_unobstructed_bounds(s_window_layer);
  prv_unobstructed_change(0, NULL);
  prv_unobstructed_will_change(now_bounds, NULL);
}

static void main_window_unload(Window *window) {
  unobstructed_area_service_unsubscribe();
  layer_destroy(s_day_layer);
  layer_destroy(s_steps_layer);
  layer_destroy(s_weather_layer);
  layer_destroy(s_sun_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_weather_text);
  text_layer_destroy(s_time_layer);
}

// —— init / deinit ——

static void init() {
  s_main_window = window_create();

  window_set_background_color(s_main_window, GColorWhite);
  window_set_window_handlers(s_main_window, (WindowHandlers){
   .load = main_window_load,
   .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  const int inbox_size = 256;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

#include "steps_dots.h"

static int s_step_goal = DEFAULT_STEP_GOAL;
static Layer *s_steps_layer_ref;

void steps_set_goal(int goal) {
  s_step_goal = goal;
}

static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate) {
    layer_mark_dirty(s_steps_layer_ref);
  }
}

void steps_init(Layer *layer) {
  s_steps_layer_ref = layer;
  health_service_events_subscribe(health_handler, NULL);
}

static GColor steps_color_for_percent(int percent) {
  if (percent >= 100) return GColorKellyGreen;
  if (percent >= 60)  return GColorChromeYellow;
  return GColorSunsetOrange;
}

void draw_steps_dots(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int y = bounds.size.h / 2;

  time_t now = time(NULL);
  int today = get_current_wday();
  GColor colors[DOT_COUNT];

  for (int i = 0; i < DOT_COUNT; i++) {
    colors[i] = GColorLightGray;

    if (i == today) {
      HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, time_start_of_today(), now);
      if (mask & HealthServiceAccessibilityMaskAvailable) {
        HealthValue steps = health_service_sum_today(HealthMetricStepCount);
        int percent = (steps * 100) / s_step_goal;
        colors[i] = steps_color_for_percent(percent);
      }
    } else if (i < today) {
      int days_ago = today - i;

      time_t day_start = time_start_of_today() - (days_ago * SECONDS_PER_DAY);
      time_t day_end = day_start + SECONDS_PER_DAY;

      HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricStepCount, day_start, day_end);
      if (mask & HealthServiceAccessibilityMaskAvailable) {
        HealthValue steps = health_service_sum(HealthMetricStepCount, day_start, day_end);
        int percent = (steps * 100) / s_step_goal;
        colors[i] = steps_color_for_percent(percent);
      }
    }
  }

  draw_standard_dots(ctx, y, colors, today);
}

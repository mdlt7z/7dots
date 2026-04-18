#pragma once
#include "dots_common.h"

#define DEFAULT_STEP_GOAL 5000

void steps_init(Layer *layer);
void steps_set_goal(int goal);
void draw_steps_dots(Layer *layer, GContext *ctx);

#pragma once

#include <string>

#define FUNC_CHEESE "cheese"
#define FUNC_CREDITS "credits"

#define RENDER_CHEESE std::string("render/" FUNC_CHEESE "/")
#define RENDER_CREDITS std::string("render/" FUNC_CREDITS "/")

void register_render_functions();

std::string credits_args(int ants, int drones, int tanks, int agents,
                         int queen_time);

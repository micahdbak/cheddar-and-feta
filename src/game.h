#pragma once

#include <cstdint>

#include "thoom/thoom.h"

struct GameDef {
  const char* title;
  bool peer;
  bool offerer;
  uint32_t background;
  const struct hud_def* hud;
  bool (*init)(int argc, char** argv);
};

bool game_run(const GameDef& def, int argc, char** argv);

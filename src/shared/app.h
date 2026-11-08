#pragma once

#include "thoom/thoom.h"

struct AppDef {
  const char* title;
  bool peer;
  bool offerer;
  unsigned long background;
  const struct hud_def* hud;
  bool (*init)(int argc, char** argv);
};

bool app_run(const AppDef& def, int argc, char** argv);

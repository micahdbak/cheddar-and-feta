#include <SDL3/SDL_main.h>

#include "constants.h"
#include "game.h"
#include "game_hud.h"
#include "guest_game.h"
#include "host_game.h"
#include "init.h"
#include "object/object.h"
#include "scene/scene.h"
#include "splash.h"
#include "thoom/thoom.h"

static const thoom::ObjectDef kInitObjects[] = {
    {INIT_OBJ, Init::create_object, thoom::kObjectNormal},
    {SPLASH_OBJ, Splash::create_object, thoom::kObjectNormal},
    {nullptr, nullptr, thoom::kObjectNormal},
};

bool init(int argc, char** argv) {
  thoom::Scene::instance->register_objects(kInitObjects);
  register_host_objects();
  register_guest_objects();
  thoom::Scene::instance->request_map("maps/splash", true);
  return true;
}

int main(int argc, char** argv) {
  struct hud_def hud = GameHud::def(true);
  hud.show_peer = init_show_peer;

  GameDef def = {"Thoom Engine", kBackground.val, &hud, init};
  return game_run(def, argc, argv) ? 0 : 1;
}

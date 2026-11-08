#include <SDL3/SDL_main.h>

#include "constants.h"
#include "game.h"
#include "game_hud.h"
#include "init.h"
#include "object.h"
#include "scene/scene.h"
#include "splash.h"
#include "thoom/thoom.h"

static const thoom::Scene::ObjectDef kInitObjects[] = {
    {INIT_OBJ, Init::create_object, false, thoom::kObjectNormal},
    {SPLASH_OBJ, Splash::create_object, false, thoom::kObjectNormal},
    {nullptr, nullptr, false, thoom::kObjectNormal},
};

bool init(int argc, char** argv) {
  thoom::Scene::instance->register_objects(kInitObjects);
  thoom::Scene::instance->request_map("maps/splash");
  return true;
}

int main(int argc, char** argv) {
  struct hud_def hud = GameHud::def(true);
  hud.show_peer = init_show_peer;

  GameDef def = {"Thoom Engine", true, false, kBackground.val, &hud, init};
  return game_run(def, argc, argv) ? 0 : 1;
}

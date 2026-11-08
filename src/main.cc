#include <SDL3/SDL_main.h>

#include "constants.h"
#include "game.h"
#include "game_hud.h"
#include "launch.h"
#include "object.h"
#include "scene/scene.h"
#include "thoom/thoom.h"

#define LAUNCH_OBJ "launch"

static const thoom::Scene::ObjectDef kLaunchObjects[] = {
    {LAUNCH_OBJ, Launch::create_object, thoom::kObjectLast},
    {nullptr, nullptr},
};

bool init(int argc, char** argv) {
  thoom::Scene::instance->register_objects(kLaunchObjects);
  thoom::Scene::instance->create_object(LAUNCH_OBJ, "");
  thoom::Scene::instance->request_map("maps/init", false);
  return true;
}

int main(int argc, char** argv) {
  struct hud_def hud = GameHud::def(true);
  hud.show_peer = launch_show_peer;

  GameDef def = {"Thoom Engine", true, false, kBackground.val, &hud, init};
  return game_run(def, argc, argv) ? 0 : 1;
}

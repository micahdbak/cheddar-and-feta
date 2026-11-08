#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>
#include <unordered_map>

#include "colour.h"
#include "constants.h"
#include "game.h"
#include "game_hud.h"
#include "guest_feta.h"
#include "guest_peer_receiver.h"
#include "guest_peer_sender.h"
#include "item.h"
#include "launch.h"
#include "object.h"
#include "scene/scene.h"
#include "thoom/thoom.h"
#include "window/window.h"

#define INIT_OBJ "load_save"

std::unordered_map<std::string, Item> item_info;

bool init(int argc, char** argv);

int main(int argc, char** argv) {
  struct hud_def hud = GameHud::def(false);
  GameDef def = {"Thoom Engine", true, true, kBackground.val, &hud, init};
  return game_run(def, argc, argv) ? 0 : 1;
}

static const thoom::Scene::ObjectDef kFetaObjects[] = {
    {INIT_OBJ, Launch::create_object},
    {PEER_RECEIVER_OBJ, GuestPeerReceiver::create_object, thoom::kObjectFirst},
    {PEER_SENDER_OBJ, GuestPeerSender::create_object, thoom::kObjectLast},
    {FETA_OBJ, GuestFeta::create_object},
    {nullptr, nullptr},
};

bool init(int argc, char** argv) {
  thoom::Scene::instance->register_objects(kFetaObjects);

  // items
  item_info[ITEM_NONE] = Item{HELD_EFFECT, "Kick"};
  item_info[ITEM_NONE].damage = 1;
  item_info[ITEM_NONE].armour = 0;
  item_info[ITEM_CANNON_BALL] = Item{THROWABLE, "Cannon Ball"};
  item_info[ITEM_CHEESE] = Item{EDIBLE, "Cheese"};
  item_info[ITEM_COFFEE_BEAN] = Item{USEFUL, "Coffee Bean"};
  item_info[ITEM_FIRE] = Item{THROWABLE, "Fire"};
  item_info[ITEM_HERMES_BOOT] = Item{HELD_EFFECT, "Hermes Boot"};
  item_info[ITEM_HERMES_BOOT].damage = 0;
  item_info[ITEM_HERMES_BOOT].speed = 1.75f;
  item_info[ITEM_MOLOTOV] = Item{THROWABLE, "Molotov Cocktail"};
  item_info[ITEM_SHIELD] = Item{HELD_EFFECT, "Shield"};
  item_info[ITEM_SHIELD].damage = 0;
  item_info[ITEM_SHIELD].armour = 1;
  item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Porcu' Pine"};

  thoom::Window::instance->set_title("Playing as Feta");

  thoom::Scene::instance->create_object(PEER_RECEIVER_OBJ, "");
  thoom::Scene::instance->create_object(PEER_SENDER_OBJ, "");

  thoom::Scene::instance->request_map("maps/init");
  return true;
}

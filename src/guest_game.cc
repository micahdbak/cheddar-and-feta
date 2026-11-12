#include "guest_game.h"

#include <string>
#include <unordered_map>

#include "guest_feta.h"
#include "guest_item.h"
#include "guest_peer_receiver.h"
#include "guest_peer_sender.h"
#include "object/object.h"
#include "scene/scene.h"
#include "window/window.h"

std::unordered_map<std::string, Item> guest_item_info;

static const thoom::ObjectDef kGuestObjects[] = {
    {GUEST_PEER_RECEIVER_OBJ, GuestPeerReceiver::create_object,
     thoom::kObjectFirst},
    {GUEST_PEER_SENDER_OBJ, GuestPeerSender::create_object, thoom::kObjectLast},
    {GUEST_FETA_OBJ, GuestFeta::create_object, thoom::kObjectNormal},
    {nullptr, nullptr, thoom::kObjectNormal},
};

void register_guest_objects() {
  thoom::Scene::instance->register_objects(kGuestObjects);
}

void guest_start() {
  guest_item_info[ITEM_NONE] = Item{HELD_EFFECT, "Kick"};
  guest_item_info[ITEM_NONE].damage = 1;
  guest_item_info[ITEM_NONE].armour = 0;
  guest_item_info[ITEM_CANNON_BALL] = Item{THROWABLE, "Cannon Ball"};
  guest_item_info[ITEM_CHEESE] = Item{EDIBLE, "Cheese"};
  guest_item_info[ITEM_COFFEE_BEAN] = Item{USEFUL, "Coffee Bean"};
  guest_item_info[ITEM_FIRE] = Item{THROWABLE, "Fire"};
  guest_item_info[ITEM_HERMES_BOOT] = Item{HELD_EFFECT, "Hermes Boot"};
  guest_item_info[ITEM_HERMES_BOOT].damage = 0;
  guest_item_info[ITEM_HERMES_BOOT].speed = 1.75f;
  guest_item_info[ITEM_MOLOTOV] = Item{THROWABLE, "Molotov Cocktail"};
  guest_item_info[ITEM_SHIELD] = Item{HELD_EFFECT, "Shield"};
  guest_item_info[ITEM_SHIELD].damage = 0;
  guest_item_info[ITEM_SHIELD].armour = 1;
  guest_item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Porcu' Pine"};

  thoom::Window::instance->set_title("Playing as Feta");

  thoom::Scene::instance->create_object(GUEST_PEER_RECEIVER_OBJ, "");
  thoom::Scene::instance->create_object(GUEST_PEER_SENDER_OBJ, "");
}

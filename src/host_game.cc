#include "host_game.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "object/entity.h"
#include "object/object.h"
#include "scene/scene.h"
#include "thoom/thoom.h"
#include "window/window.h"

// objects
#include "billboard.h"
#include "cheddar.h"
#include "credits.h"
#include "foes/bat.h"
#include "foes/bell.h"
#include "foes/bug.h"
#include "foes/frog.h"
#include "foes/gate.h"
#include "foes/porcupine.h"
#include "foes/spawner.h"
#include "foes/spitter/spitter.h"
#include "freedom.h"
#include "game_over.h"
#include "host_peer_receiver.h"
#include "host_peer_sender.h"
#include "items/cannon_ball.h"
#include "items/cheese.h"
#include "items/coffee.h"
#include "items/fire.h"
#include "items/hermes_boot.h"
#include "items/item.h"
#include "items/molotov.h"
#include "items/persister.h"
#include "items/save.h"
#include "items/shield.h"
#include "items/toothpick.h"
#include "items/tossed.h"
#include "ladder.h"
#include "remote_feta.h"

static const thoom::EntityOptions kFixed = {thoom::Entity::Ownership::kFixed,
                                            THOOM_PID_INVALID};
static const thoom::EntityOptions kRoundRobin = {
    thoom::Entity::Ownership::kRoundRobin, THOOM_PID_INVALID};

static const thoom::ObjectDef kHostObjects[] = {
    {CREDITS_OBJ, Credits::create_object, thoom::kObjectNormal},
    {FREEDOM_OBJ, Freedom::create_object, thoom::kObjectNormal},
    {GAME_OVER_OBJ, GameOver::create_object, thoom::kObjectNormal},
    {ITEM_PERSISTER_OBJ, ItemPersister::create_object, thoom::kObjectNormal},
    {ITEM_SAVE USE_OBJ, ItemSaveUse::create_object, thoom::kObjectNormal},
    {PEER_RECEIVER_OBJ, HostPeerReceiver::create_object, thoom::kObjectFirst},
    {PEER_SENDER_OBJ, HostPeerSender::create_object, thoom::kObjectLast},
    {nullptr, nullptr, thoom::kObjectNormal},
};

static const thoom::EntityDef kHostEntities[] = {
    {BELL_OBJ, Bell::create_entity, kRoundRobin, thoom::kObjectNormal},
    {BILLBOARD_OBJ, Billboard::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {CHEDDAR_OBJ, Cheddar::create_entity, kFixed, thoom::kObjectNormal},
    {FETA_OBJ, RemoteFeta::create_entity, kFixed, thoom::kObjectNormal},
    {FOE_BAT_OBJ, FoeBat::create_entity, kRoundRobin, thoom::kObjectNormal},
    {FOE_BUG_OBJ, FoeBug::create_entity, kRoundRobin, thoom::kObjectNormal},
    {FOE_FROG_OBJ, FoeFrog::create_entity, kRoundRobin, thoom::kObjectNormal},
    {FOE_PORCUPINE_OBJ, FoePorcupine::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {FOE_SPITTER_OBJ, Spitter::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {GATE_OBJ, FoeGate::create_entity, kRoundRobin, thoom::kObjectNormal},
    {ITEM_CANNON_BALL DROPPED_OBJ, DroppedCannonBall::create_entity,
     kRoundRobin, thoom::kObjectNormal},
    {ITEM_CANNON_BALL USE_OBJ, ThrownCannonBall::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {ITEM_CHEESE DROPPED_OBJ, Cheese::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {ITEM_COFFEE_BEAN DROPPED_OBJ, DroppedCoffeeBean::create_entity,
     kRoundRobin, thoom::kObjectNormal},
    {ITEM_FIRE USE_OBJ, Fire::create_entity, kRoundRobin, thoom::kObjectNormal},
    {ITEM_HERMES_BOOT DROPPED_OBJ, DroppedHermesBoot::create_entity,
     kRoundRobin, thoom::kObjectNormal},
    {ITEM_MOLOTOV DROPPED_OBJ, DroppedMolotov::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {ITEM_MOLOTOV USE_OBJ, ThrownMolotov::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {ITEM_SHIELD DROPPED_OBJ, DroppedShield::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {ITEM_TOOTHPICK DROPPED_OBJ, DroppedToothpick::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {ITEM_TOOTHPICK USE_OBJ, ThrownToothpick::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {LADDER_OBJ, Ladder::create_entity, kRoundRobin, thoom::kObjectNormal},
    {SPAWNER_OBJ, FoeSpawner::create_entity, kRoundRobin, thoom::kObjectNormal},
    {TOSSED_ITEM_OBJ, TossedItem::create_entity, kRoundRobin,
     thoom::kObjectNormal},
    {nullptr, nullptr, kRoundRobin, thoom::kObjectNormal},
};

void register_host_objects() {
  thoom::Scene::instance->register_objects(kHostObjects);
  thoom::Scene::instance->register_entities(kHostEntities);
}

void host_start() {
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
  item_info[ITEM_SAVE] = Item{USEFUL, "Save Game"};
  item_info[ITEM_SHIELD] = Item{HELD_EFFECT, "Shield"};
  item_info[ITEM_SHIELD].damage = 0;
  item_info[ITEM_SHIELD].armour = 1;
  item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Porcu' Pine"};

  thoom::Window::instance->set_title("Playing as Cheddar");

  thoom::Scene::instance->create_object(PEER_RECEIVER_OBJ, "");
  thoom::Scene::instance->create_object(PEER_SENDER_OBJ, "");
}

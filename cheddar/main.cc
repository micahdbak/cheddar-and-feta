#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstddef>

#include "audio.h"
#include "colour.h"
#include "constants.h"
#include "hud.h"
#include "icon.h"
#include "input.h"
#include "map.h"
#include "object.h"
#include "render_functions.h"
#include "renderer.h"
#include "scene.h"
#include "sounds.h"
#include "thoom/thoom.h"
#include "ui_box.h"
#include "window.h"

// objects
#include "billboard.h"
#include "cheddar.h"
#include "credits.h"
#include "feta.h"
#include "foes/bat.h"
#include "foes/bell.h"
#include "foes/bug.h"
#include "foes/frog.h"
#include "foes/gate.h"
#include "foes/porcupine.h"
#include "foes/spawner.h"
#include "foes/spitter/abdomen.h"
#include "foes/spitter/segment.h"
#include "foes/spitter/spitter.h"
#include "freedom.h"
#include "game_over.h"
#include "hitbox.h"
#include "hurtbox.h"
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
#include "load_save.h"
#include "peer_receiver.h"
#include "peer_sender.h"
#include "splash.h"

bool game_is_cheddar = true;

bool init(int argc, char** argv);

int main(int argc, char** argv) {
  if (!thoom_init("Thoom Engine", THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT,
                  false)) {
    return 1;
  }

  register_render_functions();

  if (!thoom::Scene::init()) {
    thoom_quit();
    return 1;
  }

  if (!init(argc, argv)) {
    thoom::Scene::quit();
    thoom_quit();
    return 1;
  }

  load_ui_box();
  load_icons();
  Hud::init();

  load_sounds();

  bool running = true;

  while (running) {
    bool quit = false;
    thoom_poll_events(&quit);

    if (quit) {
      running = false;
      break;
    }

    thoom::Audio::instance->step();
    thoom::Scene::instance->step();

    thoom::Map* map = thoom::Scene::instance->map;
    thoom::Renderer::instance->draw_world(map);

    if (thoom::Input::instance->world.character == 'p') {
      thoom::Window::instance->save_screenshot("screenshot.bmp");
    }

    Hud::instance->render();

    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->ui, NULL, NULL);
    thoom::Renderer::instance->draw_texture(thoom::Window::instance->screen,
                                            Hud::instance->overlay, NULL, NULL);

    thoom::Input::instance->clear_hit_input();

    thoom::Colour bg = map != nullptr ? map->background : thoom::kBlack;
    thoom::Window::instance->present(bg);
  }

  Hud::shutdown();

  thoom::Scene::quit();

  SDL_DestroyTexture(ui_box);
  ui_box = nullptr;

  SDL_DestroyTexture(icons);
  icons = nullptr;

  thoom_quit();

  return 0;
}

static const thoom::Scene::ObjectDef kCheddarObjects[] = {
    {LOAD_SAVE_OBJ, LoadSave::create_object},
    {BELL_OBJ, Bell::create_object},
    {BILLBOARD_OBJ, Billboard::create_object},
    {CHEDDAR_OBJ, Cheddar::create_object},
    {CREDITS_OBJ, Credits::create_object},
    {FETA_OBJ, Feta::create_object},
    {FREEDOM_OBJ, Freedom::create_object},
    {FOE_BAT_OBJ, FoeBat::create_object},
    {FOE_BUG_OBJ, FoeBug::create_object},
    {FOE_FROG_OBJ, FoeFrog::create_object},
    {FOE_PORCUPINE_OBJ, FoePorcupine::create_object},
    {FOE_SPITTER_ABDOMEN_OBJ, SpitterAbdomen::create_object},
    {FOE_SPITTER_SEGMENT_OBJ, SpitterSegment::create_object},
    {FOE_SPITTER_OBJ, Spitter::create_object},
    {GAME_OVER_OBJ, GameOver::create_object},
    {GATE_OBJ, FoeGate::create_object},
    {HITBOX_OBJ, HitBox::create_object},
    {HURTBOX_OBJ, HurtBox::create_object},
    {ITEM_CANNON_BALL DROPPED_OBJ, DroppedCannonBall::create_object},
    {ITEM_CANNON_BALL USE_OBJ, ThrownCannonBall::create_object},
    {ITEM_CHEESE DROPPED_OBJ, Cheese::create_object},
    {ITEM_COFFEE_BEAN DROPPED_OBJ, DroppedCoffeeBean::create_object},
    {ITEM_COFFEE_BEAN USE_OBJ, UsedCoffeeBean::create_object},
    {ITEM_FIRE USE_OBJ, Fire::create_object},
    {ITEM_HERMES_BOOT DROPPED_OBJ, DroppedHermesBoot::create_object},
    {ITEM_MOLOTOV DROPPED_OBJ, DroppedMolotov::create_object},
    {ITEM_MOLOTOV USE_OBJ, ThrownMolotov::create_object},
    {ITEM_PERSISTER_OBJ, ItemPersister::create_object},
    {ITEM_SAVE USE_OBJ, ItemSaveUse::create_object},
    {ITEM_SHIELD DROPPED_OBJ, DroppedShield::create_object},
    {ITEM_TOOTHPICK DROPPED_OBJ, DroppedToothpick::create_object},
    {ITEM_TOOTHPICK USE_OBJ, ThrownToothpick::create_object},
    {LADDER_OBJ, Ladder::create_object},
    {SPAWNER_OBJ, FoeSpawner::create_object},
    {SPLASH_OBJ, Splash::create_object},
    {TOSSED_ITEM_OBJ, TossedItem::create_object},
    {PEER_RECEIVER_OBJ, PeerReceiver::create_object, thoom::kObjectFirst},
    {PEER_SENDER_OBJ, PeerSender::create_object, thoom::kObjectLast},
    {nullptr, nullptr},
};

bool init(int argc, char** argv) {
  thoom::Scene::instance->register_objects(kCheddarObjects);

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

  thoom::Scene::instance->request_map("maps/splash");
  return true;
}

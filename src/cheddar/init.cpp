#include "game.h"
#include "controller.h"
#include "net_agent.h"
#include "object.h"
#include "save_data.h"
#include "textbox.h"
#include "audio_playback.h"

// objects
#include "billboard.h"
#include "credits.h"
#include "freedom.h"
#include "foes/bat.h"
#include "foes/bell.h"
#include "foes/bug.h"
#include "foes/frog.h"
#include "foes/gate.h"
#include "foes/mole.h"
#include "foes/porcupine.h"
#include "foes/spawner.h"
#include "foes/spitter/abdomen.h"
#include "foes/spitter/segment.h"
#include "foes/spitter/spitter.h"
#include "game_over.h"
#include "hitbox.h"
#include "hurtbox.h"
#include "items/cannon_ball.h"
#include "items/cheese.h"
#include "items/coffee.h"
#include "items/fire.h"
#include "items/frog_tongue.h"
#include "items/hermes_boot.h"
#include "items/molotov.h"
#include "items/persister.h"
#include "items/save.h"
#include "items/shield.h"
#include "items/toothpick.h"
#include "items/tossed.h"
#include "ladder.h"
#include "load_save.h"
#include "mouse.h"
#include "net_receiver.h"
#include "net_sender.h"
#include "splash.h"

#include "items/item.h"

void Game::init() {
    this->factories[LOAD_SAVE_OBJ] = new LoadSaveFactory();

    // objects
    this->factories[BELL_OBJ] = new BellFactory();
    this->factories[BILLBOARD_OBJ] = new BillboardFactory();
    this->factories[CREDITS_OBJ] = new CreditsFactory();
    this->factories[FREEDOM_OBJ] = new FreedomFactory();
    this->factories[FOE_BAT_OBJ] = new FoeBatFactory();
    this->factories[FOE_BUG_OBJ] = new FoeBugFactory();
    this->factories[FOE_FROG_OBJ] = new FoeFrogFactory();
    this->factories[FOE_MOLE_OBJ] = new FoeMoleFactory();
    this->factories[FOE_PORCUPINE_OBJ] = new FoePorcupineFactory();
    this->factories[FOE_SPITTER_ABDOMEN_OBJ] = new SpitterAbdomenFactory();
    this->factories[FOE_SPITTER_SEGMENT_OBJ] = new SpitterSegmentFactory();
    this->factories[FOE_SPITTER_OBJ] = new SpitterFactory();
    this->factories[GAME_OVER_OBJ] = new GameOverFactory();
    this->factories[GATE_OBJ] = new FoeGateFactory();
    this->factories[HITBOX_OBJ] = new HitBoxFactory();
    this->factories[HURTBOX_OBJ] = new HurtBoxFactory();
    this->factories[ITEM_CANNON_BALL DROPPED_OBJ] = new DroppedCannonBallFactory();
    this->factories[ITEM_CANNON_BALL USE_OBJ] = new ThrownCannonBallFactory();
    this->factories[ITEM_CHEESE DROPPED_OBJ] = new CheeseFactory();
    this->factories[ITEM_COFFEE_BEAN DROPPED_OBJ] = new DroppedCoffeeBeanFactory();
    this->factories[ITEM_COFFEE_BEAN USE_OBJ] = new UsedCoffeeBeanFactory();
    this->factories[ITEM_FIRE USE_OBJ] = new FireFactory();
    this->factories[ITEM_FROG_TONGUE DROPPED_OBJ] = new DroppedFrogTongueFactory();
    this->factories[ITEM_FROG_TONGUE USE_OBJ] = new FrogTongueFactory();
    this->factories[ITEM_HERMES_BOOT DROPPED_OBJ] = new DroppedHermesBootFactory();
    this->factories[ITEM_MOLOTOV DROPPED_OBJ] = new DroppedMolotovFactory();
    this->factories[ITEM_MOLOTOV USE_OBJ] = new ThrownMolotovFactory();
    this->factories[ITEM_PERSISTER_OBJ] = new ItemPersisterFactory();
    this->factories[ITEM_SAVE USE_OBJ] = new ItemSaveUseFactory();
    this->factories[ITEM_SHIELD DROPPED_OBJ] = new DroppedShieldFactory();
    this->factories[ITEM_TOOTHPICK DROPPED_OBJ] = new DroppedToothpickFactory();
    this->factories[ITEM_TOOTHPICK USE_OBJ] = new ThrownToothpickFactory();
    this->factories[LADDER_OBJ] = new LadderFactory();
    this->factories[MOUSE_OBJ] = new MouseFactory();
    this->factories[SPAWNER_OBJ] = new FoeSpawnerFactory();
    this->factories[SPLASH_OBJ] = new SplashFactory();
    this->factories[TOSSED_ITEM_OBJ] = new TossedItemFactory();

    // items
    item_info[ITEM_NONE] = Item{HELD_EFFECT, "Kick", .damage = 1, .armour = 0};
    item_info[ITEM_CANNON_BALL] = Item{THROWABLE, "Cannon Ball"};
    item_info[ITEM_CHEESE] = Item{EDIBLE, "Cheese"};
    item_info[ITEM_COFFEE_BEAN] = Item{USEFUL, "Coffee Bean"};
    item_info[ITEM_FIRE] = Item{THROWABLE, "Fire"};
    item_info[ITEM_FROG_TONGUE] = Item{THROWABLE, "Frog Tongue"};
    item_info[ITEM_HERMES_BOOT] = Item{HELD_EFFECT, "Hermes Boot", .damage = 0, .speed = 1.75f};
    item_info[ITEM_MOLOTOV] = Item{THROWABLE, "Molotov Cocktail"};
    item_info[ITEM_SAVE] = Item{USEFUL, "Save Game"};
    item_info[ITEM_SHIELD] = Item{HELD_EFFECT, "Shield", .damage = 0, .armour = 1};
    item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Porcu' Pine"};

    this->factories[FIRST_OBJ] = new NetReceiverFactory();
    this->factories[LAST_OBJ] = new NetSenderFactory();

    this->title = "Playing as Cheddar";

    net_agent = new NetworkAgent(false);

    this->create_object(FIRST_OBJ, "");
    this->create_object(LAST_OBJ, "");

    this->load_map("maps/splash");
}

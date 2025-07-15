#include "game.h"
#include "controller.h"
#include "net_agent.h"
#include "object.h"
#include "save_data.h"
#include "textbox.h"

// objects
#include "billboard.h"
#include "cheese.h"
#include "foes/bat.h"
#include "foes/bell.h"
#include "foes/bug.h"
#include "foes/frog.h"
#include "foes/gate.h"
#include "foes/porcupine.h"
#include "foes/spawner.h"
#include "items/coffee.h"
#include "items/fire.h"
#include "items/frog_tongue.h"
#include "items/grub.h"
#include "items/molotov.h"
#include "items/save.h"
#include "items/spine.h"
#include "items/toothpick.h"
#include "ladder.h"
#include "mouse.h"
#include "net_receiver.h"
#include "net_sender.h"

#include "items/item.h"

#define INIT_OBJ "init"

class Init : public Object {
public:
    Init();
    ~Init() = default;

    void step();

    std::vector<std::string> summaries;
    int sel_save = 0;
    bool render = true;
};

class InitFactory : public ObjectFactory {
public:
    InitFactory() = default;
    ~InitFactory() = default;

    Object *create(const std::string &options) override {
        return new Init();
    }
};

void Game::init() {
    this->factories[INIT_OBJ] = new InitFactory();

    // objects
    this->factories[BELL_OBJ] = new BellFactory();
    this->factories[BILLBOARD_OBJ] = new BillboardFactory();
    this->factories[CHEESE_OBJ] = new CheeseFactory();
    this->factories[FOE_BAT_OBJ] = new FoeBatFactory();
    this->factories[FOE_BUG_OBJ] = new FoeBugFactory();
    this->factories[FOE_FROG_OBJ] = new FoeFrogFactory();
    this->factories[FOE_PORCUPINE_OBJ] = new FoePorcupineFactory();
    this->factories[GATE_OBJ] = new FoeGateFactory();
    this->factories[SPAWNER_OBJ] = new FoeSpawnerFactory();
    this->factories[ITEM_COFFEE_BEAN DROPPED_OBJ] = new DroppedCoffeeBeanFactory();
    this->factories[ITEM_COFFEE_BEAN USE_OBJ] = new UsedCoffeeBeanFactory();
    this->factories[ITEM_FIRE USE_OBJ] = new FireFactory();
    this->factories[ITEM_FROG_TONGUE DROPPED_OBJ] = new DroppedFrogTongueFactory();
    this->factories[ITEM_FROG_TONGUE USE_OBJ] = new FrogTongueFactory();
    this->factories[ITEM_GRUB DROPPED_OBJ] = new DroppedGrubFactory();
    this->factories[ITEM_GRUB USE_OBJ] = new ThrownGrubFactory();
    this->factories[ITEM_MOLOTOV DROPPED_OBJ] = new DroppedMolotovFactory();
    this->factories[ITEM_MOLOTOV USE_OBJ] = new ThrownMolotovFactory();
    this->factories[ITEM_SAVE USE_OBJ] = new ItemSaveUseFactory();
    this->factories[ITEM_SPINE USE_OBJ] = new ThrownSpineFactory();
    this->factories[ITEM_TOOTHPICK DROPPED_OBJ] = new DroppedToothpickFactory();
    this->factories[ITEM_TOOTHPICK USE_OBJ] = new ThrownToothpickFactory();
    this->factories[LADDER_OBJ] = new LadderFactory();
    this->factories[MOUSE_OBJ] = new MouseFactory();

    // items
    item_info[ITEM_NONE] = Item{WEAPON, "Nothing", 1, 0};
    item_info[ITEM_COFFEE_BEAN] = Item{USEFUL, "Coffee Bean", 0, 0};
    item_info[ITEM_FIRE] = Item{THROWABLE, "Fire", 0, 0};
    item_info[ITEM_FROG_TONGUE] = Item{THROWABLE, "Frog Tongue", 0, 0};
    item_info[ITEM_GRUB] = Item{THROWABLE, "Grub", 0, 0};
    item_info[ITEM_MOLOTOV] = Item{THROWABLE, "Molotov Cocktail", 0, 0};
    item_info[ITEM_SAVE] = Item{USEFUL, "Save Game", 0, 0};
    item_info[ITEM_SPINE] = Item{THROWABLE, "Spine", 0, 0};
    item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Toothpick", 0, 0};

    this->factories[FIRST_OBJ] = new NetReceiverFactory();
    this->factories[LAST_OBJ] = new NetSenderFactory();

    this->title = "Playing as Cheddar";

    net_agent = new NetworkAgent(false);

    this->create_object(FIRST_OBJ, "");
    this->create_object(LAST_OBJ, "");

    this->load_map("maps/init");
    this->display_overlay = false;
}

// ---- init object ----

#define PADDING   8
#define UI_WIDTH  (128 + (2*PADDING))
#define UI_HEIGHT ((16*(NUM_SAVE_FILES+2)) + (2*PADDING))

Init::Init() {
    this->summaries = save.file_summaries();
    game->displaying_load_screen = true;
}

void Init::step() {
    if (textbox != nullptr) {
        textbox->step();

        if (textbox->done && local_controller.is_hit(PRIMARY)) {
            delete textbox;
            textbox = nullptr;
            this->render = true;
        } else return;
    } else if (local_controller.is_hit(PRIMARY)) {
        int ret = save.load_file(this->sel_save);
        save.puti(LOAD_SAVE, 1);
        save.puti(SAVE_FILE, this->sel_save);

        if (/* ret == LOAD_SUCCESS || ret == LOAD_NEW */ true) {
            if (!save.has(LOAD_MAP))
                save.data[LOAD_MAP] = "maps/demo00";

            game->map = save.data[LOAD_MAP];
        } else {
            textbox = new Textbox("Save file is corrupted.", INIT_OBJ, DEFAULT_FONT, 50);
        }

        return;
    }

    int diff = local_controller.is_hit(DOWN) - local_controller.is_hit(UP);
    if (diff != 0) {
        this->sel_save += diff;
        this->sel_save = cnf_clamp(this->sel_save, 0, NUM_SAVE_FILES - 1);
        this->render = true;
    }

    if (this->render) {
        game->draw_rect(game->ui, NULL, 96, 128, 160, 255, SDL_BLENDMODE_NONE);

        const int x = (SCREEN_WIDTH - UI_WIDTH) / 2;
        const int y = (SCREEN_HEIGHT - UI_HEIGHT) / 2 - 8;
        SDL_FRect ui_rect = { float(x), float(y), UI_WIDTH, UI_HEIGHT };
        game->draw_ui_box(game->ui, BOX_CONTAINER, &ui_rect);
        game->draw_text(game->ui, "--- Cheddar & Feta ---", SMALL_FONT, x + PADDING + 2, y + PADDING, 0);

        for (int i = 0; i < NUM_SAVE_FILES; i++) {
            const int save_y = y + PADDING + (16*i) + 16;

            SDL_FRect highlight_rect;
            highlight_rect.x = float(x + PADDING - 2);
            highlight_rect.y = float(save_y);
            highlight_rect.w = float(4 + UI_WIDTH - (2*PADDING));
            highlight_rect.h = 14.0f;

            game->draw_ui_box(game->ui, this->sel_save == i ? BOX_OUT_SEL : BOX_OUT, &highlight_rect);
            game->draw_text(game->ui, this->summaries[i], this->sel_save == i ? BOLD_FONT : DEFAULT_FONT, x + PADDING + 2, save_y + 2, 0);
        }

        game->draw_text(game->ui, "[^/}] to select; [Enter] to load", SMALL_FONT, x + PADDING + 2, y + UI_HEIGHT - PADDING - 8, 0);
        this->render = false;
    }
}

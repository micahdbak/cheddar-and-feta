#include "game.h"
#include "controller.h"
#include "object.h"
#include "save_data.h"
#include "textbox.h"

// objects
#include "cheese.h"
#include "enemy_bug.h"
#include "item_toothpick.h"
#include "mouse.h"
#include "save_station.h"

#include "item.h"

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
    this->factories[CHEESE_OBJ] = new CheeseFactory();
    this->factories[ENEMY_BUG_OBJ] = new EnemyBugFactory();
    this->factories[ITEM_TOOTHPICK DROPPED_OBJ] = new DroppedToothpickFactory();
    this->factories[ITEM_TOOTHPICK USE_OBJ] = new ThrownToothpickFactory();
    this->factories[MOUSE_OBJ] = new MouseFactory();
    this->factories[SAVE_STATION_OBJ] = new SaveStationFactory();

    // items
    item_info[ITEM_NONE] = Item{WEAPON, "Nothing", 1, 0};
    item_info[ITEM_TOOTHPICK] = Item{THROWABLE, "Toothpick", 0, 0};

    this->create_object(INIT_OBJ, "");
    this->title = "Cheddar n' Feta";
}

// ---- init object ----

#define PADDING   8
#define UI_WIDTH  (128 + (2*PADDING))
#define UI_HEIGHT ((16*(NUM_SAVE_FILES+2)) + (2*PADDING))

Init::Init() {
    this->summaries = save.file_summaries();
}

void Init::step() {
    if (textbox != nullptr) {
        textbox->step();

        if (textbox->done && all_inputs.is_hit(PRIMARY)) {
            delete textbox;
            textbox = nullptr;
            this->render = true;
        } else return;
    } else if (all_inputs.is_hit(PRIMARY)) {
        int ret = save.load_file(this->sel_save);

        if (/* ret == LOAD_SUCCESS || ret == LOAD_NEW */ true) {
            if (!save.data.contains("map"))
                save.data["map"] = "maps/house.map";

            game->map = save.data["map"];
        } else {
            textbox = new Textbox("Save file is corrupted.", INIT_OBJ, DEFAULT_FONT, 50);
        }

        return;
    }

    int diff = all_inputs.is_hit(DOWN) - all_inputs.is_hit(UP);
    if (diff != 0) {
        this->sel_save += diff;
        this->sel_save = cnf_clamp(this->sel_save, 0, NUM_SAVE_FILES - 1);
        this->render = true;
    }

    if (this->render) {
        game->draw_rect(NULL, 96, 128, 160, 255, SDL_BLENDMODE_NONE);

        const int x = (SCREEN_WIDTH - UI_WIDTH) / 2;
        const int y = (SCREEN_HEIGHT - UI_HEIGHT) / 2 - 8;
        SDL_FRect ui_rect = { float(x), float(y), UI_WIDTH, UI_HEIGHT };
        game->draw_ui_box(BOX_CONTAINER, &ui_rect);
        game->draw_text("--- Cheddar & Feta ---", SMALL_FONT, x + PADDING + 2, y + PADDING, 0);

        for (int i = 0; i < NUM_SAVE_FILES; i++) {
            const int save_y = y + PADDING + (16*i) + 16;

            SDL_FRect highlight_rect;
            highlight_rect.x = float(x + PADDING - 2);
            highlight_rect.y = float(save_y);
            highlight_rect.w = float(4 + UI_WIDTH - (2*PADDING));
            highlight_rect.h = 14.0f;

            game->draw_ui_box(this->sel_save == i ? BOX_OUT_SEL : BOX_OUT, &highlight_rect);
            game->draw_text(this->summaries[i], this->sel_save == i ? BOLD_FONT : DEFAULT_FONT, x + PADDING + 2, save_y + 2, 0);
        }

        game->draw_text("[^/}] to select; [Enter] to load", SMALL_FONT, x + PADDING + 2, y + UI_HEIGHT - PADDING - 8, 0);
        this->render = false;
    }
}

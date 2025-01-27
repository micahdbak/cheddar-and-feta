#include "game.h"
#include "keyboard.h"
#include "object.h"
#include "save_data.h"
#include "textbox.h"

// objects
#include "enemy_bug.h"
#include "mouse.h"
#include "save_station.h"

#define INIT_OBJ "init"

#define PADDING   8
#define UI_WIDTH  (128 + (2*PADDING))
#define UI_HEIGHT ((16*(NUM_SAVE_FILES+2)) + (2*PADDING))

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

    this->factories[ENEMY_BUG_OBJ] = new EnemyBugFactory();

    this->factories[MOUSE_OBJ] = new MouseFactory();
    this->factories[SAVE_STATION_OBJ] = new SaveStationFactory();

    this->create_object(INIT_OBJ, "");
    this->title = "Cheddar n' Feta";
}

Init::Init() {
    this->summaries = save.file_summaries();
}

void Init::step() {
    if (textbox != nullptr) {
        textbox->step();

        if (textbox->done && keyboard.is_hit(SDLK_RETURN)) {
            delete textbox;
            textbox = nullptr;
            this->render = true;
        } else return;
    } else if (keyboard.is_hit(SDLK_RETURN)) {
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

    int diff = keyboard.is_hit(SDLK_DOWN) - keyboard.is_hit(SDLK_UP);
    if (diff != 0) {
        this->sel_save += diff;
        this->sel_save = clamp(this->sel_save, 0, NUM_SAVE_FILES - 1);
        this->render = true;
    }

    if (this->render) {
        game->draw_rect(NULL, 96, 128, 160, 255, SDL_BLENDMODE_NONE);

        const int x = (SCREEN_WIDTH - UI_WIDTH) / 2;
        const int y = (SCREEN_HEIGHT - UI_HEIGHT) / 2 - 8;
        SDL_FRect ui_rect = { float(x), float(y), UI_WIDTH, UI_HEIGHT };
        game->draw_ui_box(&ui_rect);
        game->draw_text("~* Cheddar n' Feta *~", SMALL_FONT, x + PADDING + 2, y + PADDING, 0);

        for (int i = 0; i < NUM_SAVE_FILES; i++) {
            const int save_y = y + PADDING + (16*i) + 16;

            if (this->sel_save == i) {
                SDL_FRect highlight_rect;
                highlight_rect.x = float(x + PADDING);
                highlight_rect.y = float(save_y);
                highlight_rect.w = float(UI_WIDTH - (2*PADDING));
                highlight_rect.h = 14.0f;
                game->draw_rect(&highlight_rect, 255, 255, 255, 64, SDL_BLENDMODE_BLEND);
            }

            game->draw_text(this->summaries[i], DEFAULT_FONT, x + PADDING + 2, save_y + 2, 0);
        }

        game->draw_text("[^/}] to select; [Enter] to load", SMALL_FONT, x + PADDING + 2, y + UI_HEIGHT - PADDING - 8, 0);
        this->render = false;
    }
}

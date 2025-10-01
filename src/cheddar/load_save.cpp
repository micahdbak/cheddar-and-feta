#include "game.h"
#include "controller.h"
#include "load_save.h"
#include "save_data.h"
#include "textbox.h"

#define PADDING   8
#define UI_WIDTH  (128 + (2*PADDING))
#define UI_HEIGHT ((16*(NUM_SAVE_FILES+2)) + (2*PADDING))

LoadSave::LoadSave() {
    this->summaries = save.file_summaries();
}

void LoadSave::step() {
    if (textbox != nullptr) {
        textbox->step();

        if (textbox->done && local_controller.is_hit(Button::SELECT)) {
            delete textbox;
            textbox = nullptr;
            this->render = true;
        } else return;
    } else if (local_controller.is_hit(Button::SELECT)) {
        int ret = save.load_file(this->sel_save);
        save.puti(LOAD_SAVE, 1);
        save.puti(SAVE_FILE, this->sel_save);

        if (/* ret == LOAD_SUCCESS || ret == LOAD_NEW */ true) {
            if (!save.has(LOAD_MAP))
                save.data[LOAD_MAP] = "maps/demo00";

            game->map = save.data[LOAD_MAP];
        } else {
            textbox = new Textbox("Save file is corrupted.", LOAD_SAVE_OBJ, DEFAULT_FONT, 50);
        }

        return;
    }

    int diff = local_controller.is_hit(Button::DOWN) - local_controller.is_hit(Button::UP);
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

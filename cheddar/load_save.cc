#include "game.h"
#include "controller.h"
#include "load_save.h"
#include "save_data.h"
#include "textbox.h"

#define PADDING   8
#define UI_WIDTH  (128 + (2*PADDING))
#define UI_HEIGHT (48 + (2*PADDING))

LoadSave::LoadSave() {
    this->summaries = save.file_summaries();
    this->save_end = cnf_min(this->summaries.size(), 2);
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

        if (ret != LOAD_SUCCESS && ret != LOAD_NEW) {
            textbox = new Textbox("Save file is corrupted.", LOAD_SAVE_OBJ, DEFAULT_FONT, 35);
            return;
        }

        if (save.geti(GAME_DONE)) {
            textbox = new Textbox("Cannot load this game. Create a new save.", LOAD_SAVE_OBJ, DEFAULT_FONT, 35);
            return;
        }

        save.puti(LOAD_SAVE, 1);
        save.puti(SAVE_FILE, this->sel_save);

        if (!save.has(LOAD_MAP))
            save.data[LOAD_MAP] = "maps/demo00";

        game->map = save.data[LOAD_MAP];
        return;
    }

    int diff = local_controller.is_hit(Button::DOWN) - local_controller.is_hit(Button::UP);
    if (diff != 0) {
        this->sel_save += diff;
        this->sel_save = cnf_clamp(this->sel_save, 0, this->summaries.size());

        if (this->sel_save > this->save_end) {
            this->save_start++;
            this->save_end++;
        } else if (this->sel_save < this->save_start) {
            this->save_start--;
            this->save_end--;
        }

        this->render = true;
    }

    if (this->render) {
        game->draw_rect(game->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

        const int x = (SCREEN_WIDTH - UI_WIDTH) / 2;
        const int y = (SCREEN_HEIGHT - UI_HEIGHT) / 2;
        SDL_FRect ui_rect = { float(x), float(y), UI_WIDTH, UI_HEIGHT };
        game->draw_ui_box(game->ui, BOX_CONTAINER, &ui_rect);

        for (int i = save_start; i <= save_end; i++) {
            const int save_y = y + PADDING + (16 * (i - save_start));

            SDL_FRect highlight_rect;
            highlight_rect.x = float(x + PADDING - 2);
            highlight_rect.y = float(save_y + 1);
            highlight_rect.w = float(4 + UI_WIDTH - (2*PADDING));
            highlight_rect.h = 14.0f;

            game->draw_ui_box(game->ui, this->sel_save == i ? BOX_OUT_SEL : BOX_OUT, &highlight_rect);
            std::string save_text = i >= this->summaries.size() ? NEW_SAVE_STR : this->summaries[i];
            save_text = std::to_string(i + 1) + ": " + save_text;
            game->draw_text(game->ui, save_text, DEFAULT_FONT, x + PADDING + 2, save_y + 3, 0);
        }

        if (save_start > 0) {
            game->draw_text(game->ui, "^", SMALL_FONT, (SCREEN_WIDTH / 2) - 3, y + 3, 0);
        }

        if (save_end < this->summaries.size()) {
            game->draw_text(game->ui, "}", SMALL_FONT, (SCREEN_WIDTH / 2) - 3, y + UI_HEIGHT - 9, 0);
        }

        this->render = false;
    }
}

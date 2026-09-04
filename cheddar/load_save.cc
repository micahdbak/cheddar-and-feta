#include "load_save.h"

#include "controller.h"
#include "game.h"
#include "save_data.h"
#include "textbox.h"
#include "utils.h"

#define PADDING 8
#define UI_WIDTH (128 + (2 * PADDING))
#define UI_HEIGHT (48 + (2 * PADDING))

LoadSave::LoadSave() {
  this->summaries = thoom::save.file_summaries();
  this->save_end = THOOM_MIN(this->summaries.size(), 2);
}

void LoadSave::step() {
  if (textbox != nullptr) {
    textbox->step();

    if (textbox->done &&
        thoom::local_controller.is_hit(thoom::Button::SELECT)) {
      delete textbox;
      textbox = nullptr;
      this->render = true;
    } else
      return;
  } else if (thoom::local_controller.is_hit(thoom::Button::SELECT)) {
    int ret = thoom::save.load_file(this->sel_save);

    if (ret != LOAD_SUCCESS && ret != LOAD_NEW) {
      textbox = new Textbox("Save file is corrupted.", LOAD_SAVE_OBJ,
                            DEFAULT_FONT, 35);
      return;
    }

    if (thoom::save.geti(GAME_DONE)) {
      textbox = new Textbox("Cannot load this game. Create a new save.",
                            LOAD_SAVE_OBJ, DEFAULT_FONT, 35);
      return;
    }

    thoom::save.puti(LOAD_SAVE, 1);
    thoom::save.puti(SAVE_FILE, this->sel_save);

    if (!thoom::save.has(LOAD_MAP)) thoom::save.data[LOAD_MAP] = "maps/demo00";

    thoom::game->map = thoom::save.data[LOAD_MAP];
    return;
  }

  int diff = thoom::local_controller.is_hit(thoom::Button::DOWN) -
             thoom::local_controller.is_hit(thoom::Button::UP);
  if (diff != 0) {
    this->sel_save += diff;
    this->sel_save = THOOM_CLAMP(this->sel_save, 0, this->summaries.size());

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
    thoom::game->draw_rect(thoom::game->ui, NULL, 0, 0, 0, 0,
                           SDL_BLENDMODE_NONE);

    const int x = (THOOM_SCREEN_WIDTH - UI_WIDTH) / 2;
    const int y = (THOOM_SCREEN_HEIGHT - UI_HEIGHT) / 2;
    SDL_FRect ui_rect = {float(x), float(y), UI_WIDTH, UI_HEIGHT};
    thoom::game->draw_ui_box(thoom::game->ui, BOX_CONTAINER, &ui_rect);

    for (int i = save_start; i <= save_end; i++) {
      const int save_y = y + PADDING + (16 * (i - save_start));

      SDL_FRect highlight_rect;
      highlight_rect.x = float(x + PADDING - 2);
      highlight_rect.y = float(save_y + 1);
      highlight_rect.w = float(4 + UI_WIDTH - (2 * PADDING));
      highlight_rect.h = 14.0f;

      thoom::game->draw_ui_box(thoom::game->ui,
                               this->sel_save == i ? BOX_OUT_SEL : BOX_OUT,
                               &highlight_rect);
      std::string save_text =
          i >= this->summaries.size() ? NEW_SAVE_STR : this->summaries[i];
      save_text = std::to_string(i + 1) + ": " + save_text;
      thoom::game->draw_text(thoom::game->ui, save_text, DEFAULT_FONT,
                             x + PADDING + 2, save_y + 3, 0);
    }

    if (save_start > 0) {
      thoom::game->draw_text(thoom::game->ui, "^", SMALL_FONT,
                             (THOOM_SCREEN_WIDTH / 2) - 3, y + 3, 0);
    }

    if (save_end < this->summaries.size()) {
      thoom::game->draw_text(thoom::game->ui, "}", SMALL_FONT,
                             (THOOM_SCREEN_WIDTH / 2) - 3, y + UI_HEIGHT - 9,
                             0);
    }

    this->render = false;
  }
}

#include "init_host.h"

#include <string>

#include "colour.h"
#include "constants.h"
#include "input/controller.h"
#include "input/input.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "textbox.h"
#include "thoom/macros.h"
#include "window/window.h"

#define PADDING 8
#define UI_WIDTH (128 + (2 * PADDING))
#define UI_HEIGHT (48 + (2 * PADDING))

void InitHost::start(SDL_Texture* ui) {
  this->ui = ui;
  this->summaries = thoom::save.file_summaries();
  this->sel_save = 0;
  this->save_start = 0;
  this->save_end = THOOM_MIN(this->summaries.size(), 2);
  this->redraw = true;
}

void InitHost::step() {
  if (textbox != nullptr) {
    textbox->step();

    if (textbox->done &&
        thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
      delete textbox;
      textbox = nullptr;
      this->redraw = true;
    } else
      return;
  } else if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    int ret = thoom::save.load_file(this->sel_save);

    if (ret != LOAD_SUCCESS && ret != LOAD_NEW) {
      textbox = new Textbox("Save file is corrupted.", INIT_HOST_OBJ,
                            DEFAULT_FONT, 35);
      return;
    }

    if (thoom::save.geti(GAME_DONE)) {
      textbox = new Textbox("Cannot load this game. Create a new save.",
                            INIT_HOST_OBJ, DEFAULT_FONT, 35);
      return;
    }

    thoom::save.puti(LOAD_SAVE, 1);
    thoom::save.puti(SAVE_FILE, this->sel_save);

    if (!thoom::save.has(LOAD_MAP)) thoom::save.data[LOAD_MAP] = "maps/demo00";

    thoom::Scene::instance->request_map(thoom::save.data[LOAD_MAP], true);
    return;
  }

  int diff = thoom::Input::instance->world.consume_hit_dy();
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

    this->redraw = true;
  }
}

void InitHost::render() {
  if (textbox != nullptr) textbox->render();

  if (this->redraw) {
    thoom::Renderer::instance->clear(this->ui, thoom::kMask);

    const int x = (THOOM_SCREEN_WIDTH - UI_WIDTH) / 2;
    const int y = (THOOM_SCREEN_HEIGHT - UI_HEIGHT) / 2;
    SDL_FRect ui_rect = {float(x), float(y), UI_WIDTH, UI_HEIGHT};
    thoom::Renderer::instance->draw_ui_box(UI_BOX_CONTAINER, this->ui,
                                           &ui_rect);

    for (int i = save_start; i <= save_end; i++) {
      const int save_y = y + PADDING + (16 * (i - save_start));

      SDL_FRect highlight_rect;
      highlight_rect.x = float(x + PADDING - 2);
      highlight_rect.y = float(save_y + 1);
      highlight_rect.w = float(4 + UI_WIDTH - (2 * PADDING));
      highlight_rect.h = 14.0f;

      thoom::Renderer::instance->draw_ui_box(
          this->sel_save == i ? UI_BOX_OUT_SEL : UI_BOX_OUT, this->ui,
          &highlight_rect);
      std::string save_text =
          i >= this->summaries.size() ? NEW_SAVE_STR : this->summaries[i];
      save_text = std::to_string(i + 1) + ": " + save_text;
      thoom::Renderer::instance->draw_text(this->ui, DEFAULT_FONT, save_text,
                                           x + PADDING + 2, save_y + 3, 0,
                                           kBackground);
    }

    if (save_start > 0) {
      thoom::Renderer::instance->draw_text(this->ui, SMALL_FONT, "^",
                                           (THOOM_SCREEN_WIDTH / 2) - 3, y + 3,
                                           0, kBackground);
    }

    if (save_end < this->summaries.size()) {
      thoom::Renderer::instance->draw_text(this->ui, SMALL_FONT, "}",
                                           (THOOM_SCREEN_WIDTH / 2) - 3,
                                           y + UI_HEIGHT - 9, 0, kBackground);
    }

    this->redraw = false;
  }
}

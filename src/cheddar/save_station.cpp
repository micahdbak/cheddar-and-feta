#include "game.h"
#include "controller.h"
#include "mouse.h"
#include "save_data.h"
#include "save_station.h"
#include "textbox.h"

#include <iostream>

#define PADDING   8
#define UI_WIDTH  (128 + (2*PADDING))
#define UI_HEIGHT ((16*(NUM_SAVE_FILES+2)) + (2*PADDING))

SaveStation::SaveStation(int x, int y): x(x), y(y) {
    this->sprite = new Sprite("sprites/save_station.bmp", 32, 40, 250);
    this->dst_rect = { float(x), float(y - 16), 32.0f, 40.0f };
}

SaveStation::~SaveStation() {
    delete this->sprite;
    this->sprite = nullptr;
}

void SaveStation::step() {
    this->dst_rect.x = float(this->x - game->corner_x);
    this->dst_rect.y = float(this->y - game->corner_y - 16);

    this->sprite->update_frame();
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 34);

    if (textbox != nullptr && textbox->owner == SAVE_STATION_OBJ) {
        textbox->step();
        if (textbox->done && local_controller.is_hit(PRIMARY)) {
            delete textbox;
            textbox = nullptr;
            mice_locked = false;
        }

        return;
    }

    if (this->is_displaying_ui) {
        if (local_controller.is_hit(SECONDARY)) {
            this->is_displaying_ui = false;
            this->should_render = false;
            mice_locked = false;
            game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

            return;
        }
        
        if (local_controller.is_hit(PRIMARY)) {
            game->save_objects();
            save.write_file(this->sel_save);
            game->post_save_objects();
            textbox = new Textbox("Game has been saved!", SAVE_STATION_OBJ, DEFAULT_FONT, 50);
            this->is_displaying_ui = false;
            this->should_render = false;

            return;
        }

        int diff = local_controller.is_hit(DOWN) - local_controller.is_hit(UP);
        if (diff != 0) {
            this->sel_save += diff;
            this->sel_save = cnf_clamp(this->sel_save, 0, NUM_SAVE_FILES - 1);
            this->should_render = true;
        }

        // render ui
        if (this->should_render) {
            const int x = (SCREEN_WIDTH - UI_WIDTH) / 2;
            const int y = SCREEN_HEIGHT - UI_HEIGHT - PADDING;
            SDL_FRect ui_rect = { float(x), float(y), UI_WIDTH, UI_HEIGHT };
            game->draw_ui_box(BOX_CONTAINER, &ui_rect);

            game->draw_text("Save Game", DEFAULT_FONT, x + PADDING + 2, y + PADDING, 0);

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

            game->draw_text("[Enter] - save  [Escape] - cancel", SMALL_FONT, x + PADDING + 2, y + UI_HEIGHT - PADDING - 8, 0);
            this->should_render = false;
        }

        return;
    }

    // something else locked the mouse
    if (mice_locked)
        return;

    float distance = distance_between_points(this->x+16.0f, this->y+18.0f, cheddar->x, cheddar->y);
    if (distance < 32.0f) {
        this->sprite->set_animation(1);

        if (local_controller.is_hit(PRIMARY)) {
            game->draw_rect(0, 0, 0, 0, 0, SDL_BLENDMODE_NONE); // clear ui
            this->summaries = save.file_summaries();
            this->is_displaying_ui = true;
            this->should_render = true;
            this->sel_save = 0;
            mice_locked = true;
        }
    } else this->sprite->set_animation(0);
}

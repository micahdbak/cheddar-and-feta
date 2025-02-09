#include "item.h"
#include "keyboard.h"
#include "mouse.h"
#include "textbox.h"

std::unordered_map<std::string, Item> item_info;

// ---- dropped item ----

static unsigned int dropped_item_counter = 0;

DroppedItem::DroppedItem(float x, float y, const char *sprite_path, int frame_w, int frame_h, int interval_ms) {
    this->x = x;
    this->y = y;

    this->sprite = new Sprite(sprite_path, frame_w, frame_h, interval_ms);
    this->dst_rect.w = float(frame_w);
    this->dst_rect.h = float(frame_h);

    // generate a unique id for this item (for textbox updating)
    char buff[256];
    snprintf(buff, sizeof(buff), "item.dropped.%d", dropped_item_counter++);
    this->unique_id = buff;
}

DroppedItem::~DroppedItem() {
    delete this->sprite;
    this->sprite = nullptr;
}

void DroppedItem::step() {
    if (this->sprite->interval_ms > 0)
        this->sprite->update_frame();

    this->dst_rect.x = this->x - float(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - float(this->sprite->frame_h/2) - game->corner_y;
    game->push_sprite(this->sprite->texture, &this->sprite->frame, &this->dst_rect, this->sprite->frame_h/2);

    if (this->state != IDLE && (textbox == nullptr || textbox->owner != this->unique_id))
        this->state = IDLE; // don't know how that happened, but GTFO that state))

    switch (this->state) {
    case IDLE: {
        // don't check anything if the mouse is locked
        if (mouse->locked)
            return;

        float _x = this->x + float(this->sprite->frame_w/2);
        float _y = this->y + float(this->sprite->frame_h/2);

        float distance = distance_between_points(_x, _y, mouse->x, mouse->y);
        
        if (distance < 32.0f && keyboard.is_hit(SDLK_RETURN)) {
            game->draw_rect(0, 0, 0, 0, 0, SDL_BLENDMODE_NONE); // clear ui
            textbox = new Textbox(this->prompt_text(), this->unique_id, DEFAULT_FONT, 50);
            this->state = PROMPT;
            mouse->locked = true;
        }
    } break;

    case PROMPT:
        textbox->step();

        if (textbox->done) {
            this->state = CHOICE;
            this->choice = TAKE;
            this->render_choice();
        }

        break;

    case CHOICE:
        if (keyboard.is_hit(SDLK_LEFT) || keyboard.is_hit(SDLK_RIGHT)) {
            this->choice = this->choice == TAKE ? LEAVE : TAKE;
            this->render_choice();
        }

        if (keyboard.is_hit(SDLK_RETURN)) {
            delete textbox;
            std::string result_text;

            if (this->choice == TAKE) {
                std::pair<bool, std::string> result = this->take();
                if (!result.first) {
                    this->choice = LEAVE;
                }

                result_text = result.second;
            } else {
                result_text = this->leave_text();
            }

            textbox = new Textbox(result_text, this->unique_id, DEFAULT_FONT, 50);
            this->state = RESULT;
        }

        break;

    case RESULT:
        textbox->step();

        if (textbox->done && keyboard.is_hit(SDLK_RETURN)) {
            delete textbox;
            textbox = nullptr;
            mouse->locked = false;
            this->state = IDLE;

            if (this->choice == TAKE)
                game->delete_object = true;
        }

        break;
    }
}

void DroppedItem::render_choice() {
    SDL_FRect take_rect = { 126.0f, 212.0f, 32.0f, 14.0f };
    SDL_FRect leave_rect = { 160.0f, 212.0f, 36.0f, 14.0f };
    SDL_FRect text_arrow_rect = { 224.0f, 208.0f, 16.0f, 16.0f };

    game->draw_rect(&text_arrow_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);

    game->draw_ui_box(this->choice == TAKE ? BOX_OUT_SEL : BOX_OUT, &take_rect);
    game->draw_text("Take it", SMALL_FONT, 130, 216, 0);

    game->draw_ui_box(this->choice == LEAVE ? BOX_OUT_SEL : BOX_OUT, &leave_rect);
    game->draw_text("Leave it", SMALL_FONT, 164, 216, 0);
}

// ---- thrown item ----

ThrownItem::ThrownItem(float x, float y, int x_dir, int y_dir)
    : x(x), y(y), x_dir(x_dir), y_dir(y_dir) {
    this->thrown_ticks = game->ticks;
}

void ThrownItem::thrown_step() {
    // initial delay
    if (game->ticks - this->thrown_ticks < 125)
        return;

    // cycle through available enemies to find which is closest
    if (!enemies.empty()) {
        this->check_enemy++;
        if (this->check_enemy >= enemies.size())
            this->check_enemy = 0;

        Enemy *enemy = enemies[this->check_enemy];
        if (enemy != nullptr) {
            float enemy_x = enemy->x + float(game->tile_width/2);
            float enemy_y = enemy->y + float(game->tile_height/2);
            float distance = distance_between_points(this->x, this->y, enemy_x, enemy_y);

            // check if hit enemy
            if (distance < 16.0f) {
                this->on_hit(enemy);
                game->delete_object = true;
                return;
            }
        }
    }

    // throwable item expires after 1 s - 128 px
    if (game->ticks - this->thrown_ticks > 1125) {
        this->on_miss();
        game->delete_object = true;
        return;
    }

    float new_x = this->x + this->x_dir * (this->y_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;
    float new_y = this->y + this->y_dir * (this->x_dir != 0 ? DIAG_MULTIPLIER : 1.0f) * 128.0f * game->delta;

    // miss on collision with wall
    if (game->point_in_collider(new_x, new_y)) {
        this->on_miss();
        game->delete_object = true;
    } else {
        this->x = new_x;
        this->y = new_y;
    }
}

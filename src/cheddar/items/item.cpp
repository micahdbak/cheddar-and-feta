#include "controller.h"
#include "../foes/foe.h"
#include "items/item.h"
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

    this->dst_rect.x = this->x - (float)(int)(this->sprite->frame_w/2) - game->corner_x;
    this->dst_rect.y = this->y - (float)(int)(this->sprite->frame_h/2) - game->corner_y;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, this->sprite->frame_h/2);

    if (mice_locked)
        return;

    float _x = this->x + (float)(int)(this->sprite->frame_w/2);
    float _y = this->y + (float)(int)(this->sprite->frame_h/2);

    Mouse *mouse = closest_mouse(_x, _y, 32.0f);
    if ((mouse == cheddar && local_controller.is_hit(PRIMARY)) ||
        (mouse == feta && remote_controller.is_hit(PRIMARY))) {
        if (this->take(mouse)) {
            game->delete_object = true;
        } else {
            // play sound that inv is full?
        }
    }
}

void DroppedItem::render_choice() {
    SDL_FRect take_rect = { 126.0f, 212.0f, 32.0f, 14.0f };
    SDL_FRect leave_rect = { 160.0f, 212.0f, 36.0f, 14.0f };
    SDL_FRect text_arrow_rect = { 224.0f, 208.0f, 16.0f, 16.0f };

    game->draw_rect(game->ui, &text_arrow_rect, 0, 0, 0, 255, SDL_BLENDMODE_NONE);

    game->draw_ui_box(game->ui, this->choice == TAKE ? BOX_OUT_SEL : BOX_OUT, &take_rect);
    game->draw_text(game->ui, "Take it", SMALL_FONT, 130, 216, 0);

    game->draw_ui_box(game->ui, this->choice == LEAVE ? BOX_OUT_SEL : BOX_OUT, &leave_rect);
    game->draw_text(game->ui, "Leave it", SMALL_FONT, 164, 216, 0);
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

    // cycle through available foes to find which is closest
    if (!foes.empty()) {
        this->check_foe++;
        if (this->check_foe >= foes.size())
            this->check_foe = 0;

        Foe *foe = foes[this->check_foe];
        if (foe != nullptr) {
            float distance = distance_between_points(this->x, this->y, foe->x, foe->y);

            // check if hit foe
            if (distance < 16.0f) {
                this->on_hit(foe);
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

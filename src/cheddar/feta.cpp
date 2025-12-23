#include "feta.h"
#include "hurtbox.h"
#include "items/cheese.h"
#include "items/item.h"
#include "net_agent.h"
#include "net_sender.h"
#include "save_data.h"

Feta::Feta(int x, int y, int animation) {
    if (feta != nullptr) FATAL_ERROR
    feta = this;
    this->is_feta = true;

    this->sprite = new Sprite("sprites/feta.bmp", 32, 32, 250);
    this->sprite->set_animation(SLEEPING_ANIMATION);

    std::string items_s = save.value(FETA_OBJ MOUSE_ITEMS);
    if (!items_s.empty()) {
        this->items = Mouse::read_items(items_s);
    }

    // if loading a save, read the location from the save file
    if (save.geti(LOAD_SAVE) && save.has(FETA_OBJ MOUSE_X)) {
        this->x = save.getf(FETA_OBJ MOUSE_X);
        this->y = save.getf(FETA_OBJ MOUSE_Y);
        this->sprite->set_animation(save.geti(FETA_OBJ MOUSE_ANIMATION));
    } else {
        // cheddar will have provided the corresponding coordinates and animation
        this->x = (float)x;
        this->y = (float)y;
        this->sprite->set_animation(animation);
    }

    this->tile_x = (int)this->x / game->tile_width;
    this->tile_y = (int)this->y / game->tile_height;
    foe_path_find(this);

    this->dst_rect.w = 32.0f;
    this->dst_rect.h = 32.0f;

    // push hurtbox
    game->push_object(HURTBOX_OBJ, HurtBox::Options(this->id, -8, -8, 16, 12));

    this->is_down = false;
}

Feta::~Feta() {
    delete this->sprite;
    feta = nullptr;
}

void Feta::step() {
    if (game->net_state != NetworkAgent::State::CONNECTED) {
        this->synchronized = false;

        this->sprite->set_animation(SLEEPING_ANIMATION);
        this->dst_rect.x = this->x - 16.0f;
        this->dst_rect.y = this->y - 24.0f;
        game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);

        return;
    }

    if (!this->synchronized) {
        this->synchronize();
        this->synchronized = true;
    }

    if (!this->new_objects.empty()) {
        for (int i = 0; i < this->new_objects.size(); i++) {
            game->push_object(this->new_objects[i].first, this->new_objects[i].second);
        }

        this->new_objects.clear();
    }
    
    int new_tile_x = (int)this->x / game->tile_width;
    int new_tile_y = (int)this->y / game->tile_height;

    if (this->tile_x != new_tile_x || this->tile_y != new_tile_y) {
        this->tile_x = new_tile_x;
        this->tile_y = new_tile_y;
        foe_path_find(this);
    }

    this->sprite->set_frame(this->frame_i);
    this->sprite->set_animation(this->animation);
    this->dst_rect.x = this->x - 16.0f;
    this->dst_rect.y = this->y - 24.0f;
    game->push_sprite(this->sprite->tex_id, this->sprite->texture, &this->sprite->frame, &this->dst_rect, 22);
}

void Feta::save_data() {
    save.putf(FETA_OBJ MOUSE_X, this->x);
    save.putf(FETA_OBJ MOUSE_Y, this->y);
    save.puti(FETA_OBJ MOUSE_ANIMATION, this->sprite->animation);
    save.data[FETA_OBJ MOUSE_ITEMS] = Mouse::encode_items(this->items);
}

void Feta::attack(int damage) {
    NetSender::send_message(MSG_ATTACK, std::to_string(damage));
}

void Feta::push_item(const std::string &item_id) {
    NetSender::send_message(MSG_PUSH_ITEM, item_id);

    if (item_info.find(item_id) == item_info.end()) FATAL_ERROR

    Game::HudItem new_item;
    new_item.item_id = item_id;
    new_item.count = 1;

    if (this->items.empty()) {
        this->items.push_back(new_item);
        return;
    }

    // see if the item is already in inventory, and increment count
    int i = 0;
    for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
        if (it->item_id == item_id) {
            it->count++;
            return;
        }
    }

    // item not in inventory, push to back
    this->items.push_back(new_item);
}

void Feta::push_cheese(int amount) {
    NetSender::send_message(MSG_PUSH_CHEESE, std::to_string(amount));

    Game::HudItem new_item;
    new_item.item_id = ITEM_CHEESE;
    new_item.count = amount;

    if (this->items.empty()) {
        this->items.push_back(new_item);
        return;
    }

    // see if already has cheese, and increment by amount
    int i = 0;
    for (auto it = this->items.begin(); it != this->items.end(); ++it, ++i) {
        if (it->item_id == ITEM_CHEESE) {
            it->count += amount;
            return;
        }
    }

    // cheese not in inventory, push to back
    this->items.push_back(new_item);
}

void Feta::remove_item(const std::string &item_id) {
    for (auto it = this->items.begin(); it != this->items.end(); it++) {
        if (it->item_id == item_id && it->count > 0) {
            it->count--;
            if (it->count <= 0) {
                this->items.erase(it);
                break;
            }
        }
    }
}

void Feta::force_dance(Uint64 timeout_ms) {
    NetSender::send_message(MSG_FORCE_DANCE, std::to_string(timeout_ms));
}

void Feta::hitsource_notify() {
    NetSender::send_message(MSG_DID_HIT, std::string());
}

void Feta::synchronize() {
    char buff[1024];
    char x_str[256], y_str[256];
    std::string items_s;

    float_to_str(this->x, x_str, sizeof(x_str));
    float_to_str(this->y, y_str, sizeof(y_str));
    items_s = Mouse::encode_items(this->items);

    snprintf(buff, sizeof(buff), "%s,%s,%d %s", x_str, y_str, this->animation, items_s.c_str());

    NetSender::send_message(MSG_SYNC, std::string(buff));
}

void Feta::set_throw(int throw_x, int throw_y) {
    NetSender::send_message(MSG_THROW, std::to_string(throw_x) + ',' + std::to_string(throw_y));
}

void Feta::set_max_mov_speed(float max_mov_speed) {
    NetSender::send_message(MSG_MAX_SPEED, std::to_string(max_mov_speed));
}
#include "game.h"
#include "inventory.h"
#include "item.h"
#include "keyboard.h"
#include "mouse.h"
#include "textbox.h"

#include <iostream>

Inventory *inventory = nullptr;

Inventory::Inventory() {
    if (inventory != nullptr) {
        std::cerr << "Inventory::Inventory error: another inventory exists..?" << std::endl;
        exit(1);
    }
    inventory = this;
}

Inventory::~Inventory() {
    inventory = nullptr;
}

void Inventory::step() {
    if (this->menu != NOT_DISPLAYING && keyboard.is_hit(SDLK_TAB) && textbox == nullptr) {
        this->menu = NOT_DISPLAYING;
        mouse->locked = false;
        game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
        return;
    }

    switch (this->menu) {
    case NOT_DISPLAYING:
        // another menu is displaying - do nothing
        if (mouse->locked)
            return;

        if (keyboard.is_hit(SDLK_TAB)) {
            this->menu = BAG;
            mouse->locked = true;
            this->render_status_menu();
            this->render_bag_menu();
        }

        break;

    case BAG: {
        if (keyboard.is_hit(SDLK_RETURN) && !this->items.empty()) {
            this->menu = ITEM;
            this->sel_action = 0;
            this->render_item_menu();
            return;
        }

        int y_change = keyboard.is_hit(SDLK_DOWN) - keyboard.is_hit(SDLK_UP);
        if (y_change != 0 && !this->items.empty()) {
            this->sel_item = clamp(this->sel_item + y_change, 0, this->items.size()-1);
            this->render_bag_menu();
        }
    } break;

    case ITEM: {
        // go back
        if (keyboard.is_hit(SDLK_BACKSPACE)) {
            this->menu = BAG;
            game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            this->render_status_menu();
            this->render_bag_menu();
            return;
        }

        if (keyboard.is_hit(SDLK_RETURN)) {
            // do action
        }

        int y_change = keyboard.is_hit(SDLK_DOWN) - keyboard.is_hit(SDLK_UP);
        if (y_change != 0) {
            this->sel_action = clamp(this->sel_action + y_change, 0, NUM_ACTION_OPTIONS-1);
            this->render_item_menu();
        }
    } break;

    default:
        break;
    }
}

void Inventory::save_data() {
    // TODO
}

void Inventory::post_save_data() {
    // TODO
}

bool Inventory::push_item(const std::string &item_id) {
    if (this->items.size() == this->max_items)
        return false;

    if (!item_info.contains(item_id)) {
        std::cerr << "Inventory::push_item error: '" << item_id << "' is not a valid item." << std::endl;
        exit(1);
    }

    this->items.push_back(item_id);
    return true;
}

int Inventory::add_cheese(int amount) {
    int remaining_amount = this->max_cheese - this->cheese;

    if (remaining_amount < 1)
        return 0;

    if (remaining_amount < amount) {
        this->max_cheese += remaining_amount;
        return remaining_amount;
    }

    this->max_cheese += amount;
    return amount;
}

void Inventory::render_status_menu() {
    SDL_FRect status_rect = { 0.0f, 152.0f, 64.0f, 88.0f };
    game->draw_ui_box(BOX_CONTAINER, &status_rect);

    char status[256];
    snprintf(status, sizeof(status),
        "%s\n"
        "Level 1\n"
        "HP %d/%d\n"
        "XP 0/0\n\n"
        "%s\n"
        "~Damage %d\n"
        "%s\n"
        "~Armour %d",

        mouse->name.c_str(),
        /* level */
        mouse->health, mouse->max_health,
        /* xp */
        this->equipped_weapon.c_str(),
        mouse->damage,
        this->equipped_armour.c_str(),
        mouse->armour
    );

    game->draw_text(std::string(status), SMALL_FONT, 8, 160, 0);
}

void Inventory::render_bag_menu() {
    SDL_FRect bag_rect = { 0.0f, 0.0f, 80.0f, float(32 + 8*this->max_items) };
    game->draw_ui_box(BOX_CONTAINER, &bag_rect);

    char leading_text[256];
    snprintf(leading_text, sizeof(leading_text), "Bag:\n%d/%d Cheese", this->cheese, this->max_cheese);
    game->draw_text(std::string(leading_text), SMALL_FONT, 8, 8, 0);

    for (int i = 0; i < this->max_items; i++) {
        int y = 24 + i*8;
        SDL_FRect item_area = { 6.0f, float(y), 68.0f, 8.0f };
        game->draw_ui_box(i == this->sel_item ? BOX_OUT_SEL : BOX_UNDER, &item_area);

        if (i < this->items.size()) {
            if (!item_info.contains(this->items[i])) {
                std::cerr << "Inventory::render_bag_menu error: '" << this->items[i] << "' is not a valid item." << std::endl;
                exit(1);
            }

            std::string item = item_info[this->items[i]].name;
            game->draw_text(item, SMALL_FONT, 8, y, 0);
        }
    }
}

void Inventory::render_item_menu() {
    std::string item_id = this->items[this->sel_item];
    if (!item_info.contains(item_id)) {
        std::cerr << "Inventory::render_item_menu error: '" << item_id << "' is not a valid item." << std::endl;
        exit(1);
    }

    SDL_FRect item_rect = { 80.0f, 0.0f, 80.0f, 40.0f };
    game->draw_ui_box(BOX_CONTAINER, &item_rect);

    std::string action_text;

    switch (item_info[item_id].type) {
    case USEFUL: action_text = "Use"; break;
    case EDIBLE: action_text = "Eat"; break;
    case THROWABLE: action_text = "Next Attack"; break;
    case WEAPON: action_text = "Equip Weapon"; break;
    case ARMOUR: action_text = "Equip Armour"; break;
    }

    for (int i = 0; i < NUM_ACTION_OPTIONS; i++) {
        std::string option_text;
        switch (i) {
        case 0: option_text = action_text; break;
        case 1: option_text = "Describe"; break;
        default: option_text = "Throw Away"; break;
        }

        int y = 8 + i*8;
        SDL_FRect option_rect = { 86.0f, float(y), 68.0f, 8.0f };
        game->draw_ui_box(this->sel_action == i ? BOX_OUT_SEL : BOX_UNDER, &option_rect);
        game->draw_text(option_text, SMALL_FONT, 86, y, 0);
    }
}
